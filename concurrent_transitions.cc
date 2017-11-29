#include <map>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <thread>
#include <mutex>
#include "tbb/task_scheduler_init.h"
#include "tbb/concurrent_vector.h"
#include "FWCore/Concurrency/interface/WaitingTaskHolder.h"
#include "FWCore/Concurrency/interface/WaitingTaskList.h"
#include "FWCore/Concurrency/interface/FunctorTask.h"
#include "FWCore/Concurrency/interface/SerialTaskQueue.h"

enum class Transition {
   IsInvalid,
   IsStop,
   IsFile,
   IsRun,
   IsLumi,
   IsEvent
};

enum class Phase {
   kBeginRun,
   kBeginLumi,
   kEvent,
   kEndLumi,
   kEndRun
};


static const std::array<const char * const,5> s_phaseName = {"beginRun","beginLumi","event","endLumi","endRun"};

static std::mutex s_logMutex;

static const std::map<Transition,std::string> s_transToName = {
  {Transition::IsStop, "Stop"},
  {Transition::IsFile, "File"},
  {Transition::IsRun, "Run"},
  {Transition::IsLumi, "Lumi"},
  {Transition::IsEvent, "Event"} 
};

void printTrans( Transition iT) {
   std::lock_guard<std::mutex> g{s_logMutex};
   std::cout <<"Source: "<<s_transToName.find(iT)->second<<std::endl;
}

struct Sync {
   int m_run;
   int m_lumi;
   int m_event;
   
   bool operator==(Sync const& iOther) const {
      return m_run == iOther.m_run && m_lumi == iOther.m_lumi && m_event == iOther.m_event;
   }
   bool operator!=(Sync const& iOther) const {
      return not (*this == iOther);
   }
   
   bool operator<(Sync const& iOther) const {
      if(m_run == iOther.m_run) {
         if(m_lumi == iOther.m_lumi) {
            return m_event < iOther.m_event;
         }
         return m_lumi < iOther.m_lumi;
      }
      return m_run < iOther.m_run;
   }
};
static tbb::concurrent_vector<std::tuple<Phase,Sync,int>> s_seenSyncs;

class RunPrincipal {
public:
   RunPrincipal() {}
   
   const Sync& sync() const { return m_sync;}

   void setSync( Sync iSync) { m_sync = std::move(iSync);}
private:
   Sync m_sync;
};

class LumiPrincipal {
public:
   LumiPrincipal() {}
   
   const Sync& sync() const { return m_sync;}

   void setSync( Sync iSync) { m_sync = std::move(iSync);}
   void setRun(std::shared_ptr<RunPrincipal> iRun) { m_run = std::move(iRun);}
private:
   Sync m_sync;
   std::shared_ptr<RunPrincipal> m_run;
};

class EventPrincipal {
public:
   EventPrincipal(int iStreamID): m_id{iStreamID} {}
   
   int streamID() const {return m_id;}
   
   const Sync& sync() const { return m_sync;}

   void setSync( Sync iSync) { m_sync = std::move(iSync);}
   void setLumi(std::shared_ptr<LumiPrincipal> iLumi) { m_lumi = std::move(iLumi);}
private:
   Sync m_sync;
   std::shared_ptr<LumiPrincipal> m_lumi;
   int m_id;
};


class Source {
public:
   Source(std::vector<std::pair<Transition,Sync>> iTransitions): m_transitions(std::move(iTransitions)),
   m_nextTransition(m_transitions.begin()) {}
   
   Transition nextItemType() {
      if(m_nextTransition == m_transitions.end()) {
         return Transition::IsStop;
      }
      auto present = *(m_nextTransition++);
      m_present = present.second;
      
      printTrans(present.first);
      
      return present.first;
   }
   
   Sync sync() const { return m_present;}
private:
   std::vector<std::pair<Transition,Sync>> m_transitions;
   std::vector<std::pair<Transition,Sync>>::const_iterator m_nextTransition;
   Sync m_present;
};

class StreamSchedule {
  public:
     StreamSchedule(int iStreamID): m_streamID{iStreamID}{}
     
     void processOneEventAsync(edm::WaitingTaskHolder iTask, EventPrincipal& iEP) {
        dummyWorkAsync(std::move(iTask),Phase::kEvent, iEP.sync());
     }
     void processOneBeginLumiAsync(edm::WaitingTaskHolder iTask,  LumiPrincipal& iLumi) {
        dummyWorkAsync(std::move(iTask),Phase::kBeginLumi,iLumi.sync());
     }
     void processOneBeginRunAsync(edm::WaitingTaskHolder iTask,  RunPrincipal& iRun) {
        dummyWorkAsync(std::move(iTask),Phase::kBeginRun,iRun.sync());        
     }

     void processOneEndLumiAsync(edm::WaitingTaskHolder iTask,  LumiPrincipal& iLumi) {
        dummyWorkAsync(std::move(iTask),Phase::kEndLumi,iLumi.sync());
     }
     void processOneEndRunAsync(edm::WaitingTaskHolder iTask, RunPrincipal& iRun) {
        dummyWorkAsync(std::move(iTask),Phase::kEndRun,iRun.sync());        
     }
     
  private:
     void dummyWorkAsync(edm::WaitingTaskHolder iTask, Phase iTran, const Sync& iSync) {
        auto streamID = m_streamID;
        
        auto t = edm::make_functor_task(tbb::task::allocate_root(), [iTask,iTran,streamID,iSync]() mutable {
             using namespace std::chrono_literals;
             s_seenSyncs.emplace_back(iTran, iSync,streamID);
             {
                std::lock_guard<std::mutex> g{s_logMutex};
                std::cout <<"Stream transition "<<iSync.m_run<<" "<<iSync.m_lumi<<" "<<iSync.m_event<<" "<<s_phaseName[static_cast<int>(iTran)]<<" stream:"<<streamID<<std::endl;
             }
             std::this_thread::sleep_for(1s);
             iTask.doneWaiting(std::exception_ptr{});
             });
        if(m_streamID == 0) {
           tbb::task::spawn( *t );
        }else {
           tbb::task::enqueue( *t );
        }
     }
     const int m_streamID;
};

class GlobalSchedule {
  public:
     void processOneBeginLumiAsync(edm::WaitingTaskHolder iTask,LumiPrincipal& iLumi) {
        auto s = iLumi.sync();
        dummyWorkAsync(std::move(iTask), [s]() { 
           s_seenSyncs.emplace_back(Phase::kBeginLumi,s,-1);
           std::lock_guard<std::mutex> g{s_logMutex};
           std::cout <<"Begin Global Lumi "<<s.m_run<<" "<<s.m_lumi<<std::endl;
           });
     }
     void processOneBeginRunAsync(edm::WaitingTaskHolder iTask, RunPrincipal& iRun) {
        auto s = iRun.sync();
        dummyWorkAsync(std::move(iTask), 
        [s]() { 
             s_seenSyncs.emplace_back(Phase::kBeginRun,s,-1);
             std::lock_guard<std::mutex> g{s_logMutex};
             std::cout <<"Begin Global Run "<<s.m_run<<std::endl;
             });
     }

     void processOneEndLumiAsync(edm::WaitingTaskHolder iTask,LumiPrincipal& iLumi) {
        auto s = iLumi.sync();
        dummyWorkAsync(std::move(iTask), [s]() { 
           s_seenSyncs.emplace_back(Phase::kEndLumi,s,1000);
           std::lock_guard<std::mutex> g{s_logMutex};
           std::cout <<"End Global Lumi "<<s.m_run<<" "<<s.m_lumi<<std::endl;
           });
     }
     void processOneEndRunAsync(edm::WaitingTaskHolder iTask, RunPrincipal& iRun) {
        auto s = iRun.sync();
        dummyWorkAsync(std::move(iTask), 
        [s]() { 
             s_seenSyncs.emplace_back(Phase::kEndRun,s,1000);
             std::lock_guard<std::mutex> g{s_logMutex};
             std::cout <<"End Global Run "<<s.m_run<<std::endl;
             });
     }
     
  private:
     template <typename F>
     void dummyWorkAsync(edm::WaitingTaskHolder iTask,F && iFunc) {
        tbb::task::spawn( * edm::make_functor_task(tbb::task::allocate_root(), [iTask, iFunc]() mutable {
           using namespace std::chrono_literals;
           iFunc();
           std::this_thread::sleep_for(1s);
           iTask.doneWaiting(std::exception_ptr{});
           }));
     }
};

class PrincipalCache {
public:
   PrincipalCache(unsigned int iNStreams):
   lumiPrincipal_{std::make_shared<LumiPrincipal>()},
   runPrincipal_{std::make_shared<RunPrincipal>()}
    {
      eventPrincipals_.reserve(iNStreams);
      for(unsigned int i=0; i<iNStreams;++i) {
         eventPrincipals_.emplace_back( std::make_unique<EventPrincipal>(i));
      }
   }
   EventPrincipal& eventPrincipal(int streamID) const {return *eventPrincipals_[streamID];}
   
   std::shared_ptr<LumiPrincipal> lumiPrincipal() { return lumiPrincipal_;}
   std::shared_ptr<RunPrincipal> runPrincipal() { return runPrincipal_;}
private:
   std::vector<std::unique_ptr<EventPrincipal>> eventPrincipals_;
   std::shared_ptr<LumiPrincipal> lumiPrincipal_;
   std::shared_ptr<RunPrincipal> runPrincipal_;
};

class EventProcessor {
public:
   EventProcessor(std::vector<std::pair<Transition,Sync>> iTransitions,
   int iNStreams):
   principalCache_(iNStreams),
   m_source(std::move(iTransitions)),
   m_streamSchedules(),
   m_nStreams(iNStreams),
   nextItemTypeFromProcessingEvents_{Transition::IsInvalid} {
      m_streamSchedules.reserve(m_nStreams);
      for(int i=0; i<m_nStreams;++i) {
         m_streamSchedules.emplace_back(i);
      }
   }
   
   Transition nextTransitionType() { return m_source.nextItemType(); }
   
   //TO BE REMOVED
   Transition nextItemTypeFromProcessingEvents () const { return nextItemTypeFromProcessingEvents_;}

   Sync sync() const { return m_source.sync();}
   
   void closeInputFile() {}
   void closeOutputFiles() {}

   void readRun() {}
   void beginRunAsync(int iRunNumber, edm::WaitingTaskHolder iHolder) {
      {
         std::lock_guard<std::mutex> g{s_logMutex};
         std::cout <<" beginRun "<<iRunNumber<<std::endl;      
      }
      
      auto runP = principalCache_.runPrincipal();
      runP->setSync({iRunNumber,0,0});
      
      auto streamBeginRunTask = edm::make_waiting_task(tbb::task::allocate_root(),
      [this,iHolder,runP]( std::exception_ptr const* iPtr) {
         for(unsigned int i=0; i<m_nStreams;++i) {
            m_streamSchedules[i].processOneBeginRunAsync(iHolder,*runP);
         }         
      });
      
      m_globalSchedule.processOneBeginRunAsync(edm::WaitingTaskHolder{streamBeginRunTask},*runP);
   }
   void readAndMergeRun() {
      std::lock_guard<std::mutex> g{s_logMutex};
      std::cout <<" merge run "<<std::endl; //<<runID.m_run<<std::endl;
   }
      
   void endRun(int iRunNumber) {
      auto runP = principalCache_.runPrincipal();
      
      {
         {
            std::lock_guard<std::mutex> g{s_logMutex};
            std::cout <<"endRun "<<iRunNumber<<std::endl;
         }
         
         auto streamWaitTask = edm::make_empty_waiting_task();
         auto streamWaitTaskPtr = streamWaitTask.get();
         streamWaitTask->increment_ref_count();
         
         {
            edm::WaitingTaskHolder holdUntilAllStreamsCalled{streamWaitTask.get()};
            for(unsigned int i=0; i<m_nStreams;++i) {
               m_streamSchedules[i].processOneEndRunAsync(edm::WaitingTaskHolder(holdUntilAllStreamsCalled), *runP);
            }
         }
         streamWaitTask->wait_for_all();
      }
      auto globalWaitTask = edm::make_empty_waiting_task();
      auto globalWaitTaskPtr = globalWaitTask.get();
      globalWaitTask->increment_ref_count();
      m_globalSchedule.processOneEndRunAsync(edm::WaitingTaskHolder{globalWaitTask.get()},*runP);
      globalWaitTask->wait_for_all();      
   }
   void writeRun(int iRunNumber) {}
   void deleteRunFromCache(int iRunNumber) {}

   void readLuminosityBlock() {
      //actually creates a new lumi
      auto lumiP = principalCache_.lumiPrincipal();
      
      auto runP = principalCache_.runPrincipal(); //handed to lumi
      lumiP->setRun(std::move(runP));
   }
   void beginLumiAsync(Sync const& iSync, edm::WaitingTaskHolder iHolder, std::atomic<bool>* finishedProcessingEventsPtr) {
      {
         std::lock_guard<std::mutex> g{s_logMutex};
         std::cout <<"beginLumi "<< iSync.m_run<<" "<<iSync.m_lumi<<std::endl;
      }
      auto lumiP = principalCache_.lumiPrincipal();
      lumiP->setSync(iSync);
      
      
      auto streamBeginLumiTask = edm::make_waiting_task(tbb::task::allocate_root(),
      [this,iHolder,lumiP,finishedProcessingEventsPtr]( std::exception_ptr const* iPtr) {
         //Want to start processing events within a stream once beginStream finishes
         //we need to read from the source the next transition
         m_firstEventInBlock = false;
         nextItemTypeFromProcessingEvents_ = Transition::IsInvalid;

         for(unsigned int i=0; i<m_nStreams;++i) {
            auto eventTask = edm::make_waiting_task(tbb::task::allocate_root(),
               [this,i,iHolder,finishedProcessingEventsPtr](std::exception_ptr const* iPtr) {
                  handleNextEventForStreamAsync(iHolder,i, principalCache_.eventPrincipal(i),finishedProcessingEventsPtr);
               });

            m_streamSchedules[i].processOneBeginLumiAsync(edm::WaitingTaskHolder{eventTask},*lumiP);
         }         
      });
      
      m_globalSchedule.processOneBeginLumiAsync(edm::WaitingTaskHolder{streamBeginLumiTask},*lumiP);
   }
   void readAndMergeLumi() {
      std::lock_guard<std::mutex> g{s_logMutex};
      std::cout <<" merge lumi "<<std::endl; //<<runID.m_run<<std::endl;
   }
   void endLumi(Sync const& iSync) {
      {
         std::lock_guard<std::mutex> g{s_logMutex};
         std::cout <<"endLumi "<< iSync.m_run<<" "<<iSync.m_lumi<<std::endl;
      }
      
      auto lumiP = principalCache_.lumiPrincipal();
      
      {
         auto streamWaitTask = edm::make_empty_waiting_task();
         auto streamWaitTaskPtr = streamWaitTask.get();
         streamWaitTask->increment_ref_count();
         
         {
            edm::WaitingTaskHolder holdUntilAllStreamsCalled{streamWaitTask.get()};
            for(unsigned int i=0; i<m_nStreams;++i) {
               m_streamSchedules[i].processOneEndLumiAsync(edm::WaitingTaskHolder(holdUntilAllStreamsCalled),*lumiP);
            }
         }
         streamWaitTask->wait_for_all();
      }
      {
         auto globalWaitTask = edm::make_empty_waiting_task();
         auto globalWaitTaskPtr = globalWaitTask.get();
         globalWaitTask->increment_ref_count();
         m_globalSchedule.processOneEndLumiAsync(edm::WaitingTaskHolder{globalWaitTask.get()},*lumiP);
         globalWaitTask->wait_for_all();
      }
   }
   void writeLumi(Sync const&) {}
   void deleteLumiFromCache(Sync const&) {}
   
   Transition readAndProcessEvents() {
      m_firstEventInBlock = true;
      nextItemTypeFromProcessingEvents_ = Transition::IsEvent;
      {
         std::lock_guard<std::mutex> g{s_logMutex};
         std::cout <<"read and process events"<<std::endl;
      }
      
      std::atomic<bool> finishedProcessingEvents{false};
      auto finishedProcessingEventsPtr = &finishedProcessingEvents;
      
    //To wait, the ref count has to b 1+#streams
    auto eventLoopWaitTask = edm::make_empty_waiting_task();
    eventLoopWaitTask->increment_ref_count();

    unsigned int iStreamIndex = 0;
    for(; iStreamIndex<m_nStreams-1; ++iStreamIndex) {
      tbb::task::enqueue( *edm::make_waiting_task(tbb::task::allocate_root(),[this,iStreamIndex,h=edm::WaitingTaskHolder{eventLoopWaitTask.get()},finishedProcessingEventsPtr](std::exception_ptr const*){
        handleNextEventForStreamAsync(h,iStreamIndex, principalCache_.eventPrincipal(iStreamIndex),finishedProcessingEventsPtr);
      }) );
    }
    //need a temporary Task so that the temporary WaitingTaskHolder assigned to h will go out of scope
    // before the call to spawn_and_wait_for_all
    auto t = edm::make_waiting_task(tbb::task::allocate_root(),[this,iStreamIndex,h=edm::WaitingTaskHolder{eventLoopWaitTask.get()},finishedProcessingEventsPtr](std::exception_ptr const*){
       handleNextEventForStreamAsync(h,iStreamIndex, principalCache_.eventPrincipal(iStreamIndex),finishedProcessingEventsPtr);
       });
    eventLoopWaitTask->spawn_and_wait_for_all( *t);
    return nextItemTypeFromProcessingEvents_.load();
   }
   
private:
   
   void handleNextEventForStreamAsync(edm::WaitingTaskHolder iTask, unsigned int iStreamIndex, EventPrincipal& iEP, std::atomic<bool>* finishedProcessingEvents) {
      m_sourceQueue.push([this,iTask,iStreamIndex,&iEP,finishedProcessingEvents]() mutable {

             try {
               if(readNextEventForStream(iStreamIndex,finishedProcessingEvents) ) {
                  iEP.setSync(m_source.sync());
                  auto recursionTask = edm::make_waiting_task(tbb::task::allocate_root(), 
                                                              [this,iTask,iStreamIndex,
                                                               &iEP,finishedProcessingEvents](std::exception_ptr const* iPtr) mutable {
                    if(iPtr) {
                      iTask.doneWaiting(*iPtr);
                      //the stream will stop now
                      return;
                    }

                    handleNextEventForStreamAsync(iTask, iStreamIndex, iEP,finishedProcessingEvents);
                  });
                  
                 processEventAsync( edm::WaitingTaskHolder(recursionTask), iStreamIndex,iEP);
               } else {
                 //the stream will stop now
                 iTask.doneWaiting(std::exception_ptr{});
               }
             } catch(...) {
               iTask.doneWaiting(std::current_exception());
             }
      });
      
   }
   bool readNextEventForStream(unsigned int iStreamIndex, std::atomic<bool>* finishedProcessingEvents) {
      if(finishedProcessingEvents->load()) {
         return false;
      }
      if(not m_firstEventInBlock) {
         //The state machine already called input_->nextItemType
         // and found an event. We can't call input_->nextItemType
         // again since it would move to the next transition
         Transition itemType = m_source.nextItemType();
         if (Transition::IsEvent !=itemType) {
           nextItemTypeFromProcessingEvents_ = itemType;
           finishedProcessingEvents->store(true);
           //std::cerr<<"next item type "<<itemType<<"\n";
           return false;
        }
      } else {
         m_firstEventInBlock=false;
      }
      return true;
   }
   
   void processEventAsync(edm::WaitingTaskHolder iHolder,
                          unsigned int iStreamIndex,
                          EventPrincipal& iEP) {
     tbb::task::spawn( *edm::make_functor_task( tbb::task::allocate_root(), [iHolder,iStreamIndex,this,&iEP]() {
       processEventAsyncImpl(iHolder, iStreamIndex,iEP);
     }) );
   }
   
   void processEventAsyncImpl(edm::WaitingTaskHolder iHolder,
      unsigned int iStreamIndex, EventPrincipal& iEP) {
         auto l = principalCache_.lumiPrincipal(); //passed to event
         iEP.setLumi(std::move(l));
         m_streamSchedules[iStreamIndex].processOneEventAsync(std::move(iHolder),iEP);
   }
   
   PrincipalCache principalCache_;
   edm::SerialTaskQueue m_sourceQueue;
   Source m_source;
   GlobalSchedule m_globalSchedule;
   std::vector<StreamSchedule> m_streamSchedules;
   int m_nStreams;
   std::atomic<Transition> nextItemTypeFromProcessingEvents_;
   
   bool m_firstEventInBlock=true;
};

/* ---------------------
Transition handling
   --------------------- */
struct FileResources {
  ~FileResources() {
     std::lock_guard<std::mutex> g{s_logMutex};
     std::cout <<"file end"<<std::endl;
  } 
};

struct RunResources {
   RunResources( EventProcessor& iP, int iRun) noexcept: m_ep(iP), m_run(iRun) {}
   ~RunResources() noexcept {
      m_ep.endRun(m_run);
      //std::lock_guard<std::mutex> g{s_logMutex};
      //std::cout <<" run "<<m_run<<" end"<<std::endl;
   }
   
   Sync runID() const { return {m_run, 0, 0}; }
   
   EventProcessor& m_ep;
   int m_run;
};

struct LumiResources {
   LumiResources(std::shared_ptr<RunResources> run, int iLumi) noexcept :
   m_run(std::move(run)),
   m_lumi(iLumi) {}
   
   ~LumiResources() noexcept {
      m_run->m_ep.endLumi(lumiID());
   }
   
   Sync lumiID() const { return {m_run->m_run, m_lumi, 0}; }
   
   std::shared_ptr<RunResources> m_run;
   int m_lumi;
};


struct EventResources {
   std::shared_ptr<LumiResources> m_lumi;
   int m_event; 
};

struct EventsInLumiProcessor {
   Transition processEvents(EventProcessor& iEP, std::shared_ptr<LumiResources> iLumi) {
      return iEP.readAndProcessEvents();
   }
};

struct LumisInRunProcessor {
   Transition processLumis(EventProcessor& iEP, std::shared_ptr<RunResources> iRun) {
      bool finished = false;
      auto nextTrans = Transition::IsLumi;
      do {
         switch(nextTrans) {
            case Transition::IsLumi:
            {
               nextTrans = processLumi(iEP, iRun);
               break;
            }
            case Transition::IsEvent:
            {
               nextTrans = m_events.processEvents(iEP,m_currentLumi);
               break;
            }
            default:
            finished =true;
         }
      }while(not finished);
      return nextTrans;
   }
   
   void normalEnd() {
     m_currentLumi.reset();
   }
   
   Transition processLumi(EventProcessor& iEP, std::shared_ptr<RunResources> iRun) {
      auto lumiID =iEP.sync();
      if( (not m_currentLumi) or 
          m_currentLumi->m_run.get() != iRun.get() or 
          m_currentLumi->lumiID() != lumiID)
      {
         //switch to different lumi
         m_currentLumi = std::make_shared<LumiResources>(std::move(iRun), lumiID.m_lumi );
         iEP.readLuminosityBlock();
         auto lumiWaitTask = edm::make_empty_waiting_task();
         lumiWaitTask->increment_ref_count();
         std::atomic<bool> finishedProcessingEvents{false};
         iEP.beginLumiAsync(lumiID, edm::WaitingTaskHolder{lumiWaitTask.get()},&finishedProcessingEvents);
         lumiWaitTask->wait_for_all();
         return iEP.nextItemTypeFromProcessingEvents();
      } else {
         iEP.readAndMergeLumi();
      }
      return iEP.nextTransitionType(); 
   }
   
   std::shared_ptr<LumiResources> m_currentLumi;
   EventsInLumiProcessor m_events;
};

class RunsInFileProcessor {
public:
  Transition processRuns(EventProcessor& iEP) {
    bool finished = false;
    auto nextTransition = Transition::IsRun;
    do {
      switch(nextTransition) {
        case Transition::IsRun:
        {
          processRun(iEP);
          nextTransition = iEP.nextTransitionType();
          break;
        }
        case Transition::IsLumi:
        {
          nextTransition = lumis_.processLumis(iEP, currentRun_);
          break;
        }
        default:
          finished=true;
      }
    } while(not finished);
    return nextTransition;
  }
  
  void normalEnd() {
    lumis_.normalEnd();
    currentRun_.reset();
  }
  
private:
  void processRun(EventProcessor& iEP) {
    auto runID = iEP.sync();
    if ( (not currentRun_) or
        (currentRun_->m_run !=runID.m_run) ) {
      if(currentRun_) {
        //Both the current run and lumi end here
        lumis_.normalEnd();
      }
      currentRun_ = std::make_shared<RunResources>(iEP,runID.m_run);
      {
         //iEP.readRun();
         auto runWaitTask = edm::make_empty_waiting_task();
         runWaitTask->increment_ref_count();
         iEP.beginRunAsync(runID.m_run, edm::WaitingTaskHolder{runWaitTask.get()});         
         runWaitTask->wait_for_all();
      }
    } else {
      //merge
       iEP.readAndMergeRun();
    }
  }
  
  std::shared_ptr<RunResources> currentRun_;
  LumisInRunProcessor lumis_;
  
};


class FilesProcessor {
public:
  
  Transition processFiles(EventProcessor& iEP) {
    bool finished = false;
    auto nextTransition = iEP.nextTransitionType();
    if(nextTransition != Transition::IsFile) return nextTransition;
    do {
      switch(nextTransition) {
        case Transition::IsFile:
        {
          processFile(iEP);
          nextTransition = iEP.nextTransitionType();
          break;
        }
        case Transition::IsRun:
        {
          nextTransition = runs_.processRuns(iEP);
          break;
        }
        default:
          finished = true;
      }
    } while(not finished);
    runs_.normalEnd();
    
    return nextTransition;
  }
  
  void normalEnd() {
    runs_.normalEnd();
    if(filesOpen_) {
      filesOpen_.reset();
    }
  }
  
private:
  void processFile(EventProcessor& iEP) {
    if(not filesOpen_) {
      readFirstFile();
    } else {
      if(shouldWeCloseOutput()) {
        //Need to end this run on the file boundary
        runs_.normalEnd();
        gotoNewInputAndOutputFiles();
      } else {
        gotoNewInputFile();
      }
    }
  }
  
  void readFirstFile() {
     {
        std::lock_guard<std::mutex> g{s_logMutex};
        std::cout<<"reading first file"<<std::endl;
     }
    filesOpen_ = std::make_unique<FileResources>();
  }
  
  bool shouldWeCloseOutput() {
    //return iEP.shouldWeCloseOutput();
     return false;
  }
  
  void gotoNewInputFile() {
     std::lock_guard<std::mutex> g{s_logMutex};
     std::cout << "gotoNewInputFile"<<std::endl;
  }
  
  void gotoNewInputAndOutputFiles() {
     std::lock_guard<std::mutex> g{s_logMutex};
     std::cout << "gotoNewInputAndOutputFiles"<<std::endl;
  }
  
  std::unique_ptr<FileResources> filesOpen_;
  RunsInFileProcessor runs_;
};

/*====================================================
Testing
 ====================================================*/

std::vector<std::tuple<Phase,Sync,int>> expectedValues(std::vector<std::pair<Transition,Sync>> const& iTrans, int iNStreams ) {
   std::vector<std::tuple<Phase,Sync,int>> returnValue;
   returnValue.reserve(iTrans.size());
   
   Sync lastRun = {-1,0,0};
   Sync lastLumi = {-1,-1,0};
   for(auto const& tran: iTrans) {
      switch(tran.first) {
         case Transition::IsFile:
         {
            break;
         }
         case Transition::IsRun:
         {
            if(tran.second != lastRun) {
               if(lastRun.m_run != -1) {
                  //end transitions
                  for(int i = 0; i<iNStreams;++i) {
                     returnValue.emplace_back(Phase::kEndRun,lastRun,i);
                  }
                  returnValue.emplace_back(Phase::kEndRun,lastRun,1000);
               }
               //begin transitions
               returnValue.emplace_back(Phase::kBeginRun,tran.second,-1);
               for(int i = 0; i<iNStreams;++i) {
                  returnValue.emplace_back(Phase::kBeginRun,tran.second,i);
               }
               lastRun = tran.second;
            }
            break;
         }
         case Transition::IsLumi:
         {
            if(tran.second != lastLumi) {
               if(lastLumi.m_run != -1) {
                  //end transitions
                  for(int i = 0; i<iNStreams;++i) {
                     returnValue.emplace_back(Phase::kEndLumi,lastLumi,i);
                  }
                  returnValue.emplace_back(Phase::kEndLumi,lastLumi,1000);
               }
               //begin transitions
               returnValue.emplace_back(Phase::kBeginLumi,tran.second,-1);
               for(int i = 0; i<iNStreams;++i) {
                  returnValue.emplace_back(Phase::kBeginLumi,tran.second,i);
               }
               lastLumi = tran.second;
            }
            break;
            
         }
         case Transition::IsEvent:
         {
            returnValue.emplace_back(Phase::kEvent,tran.second,-2);
         }
         case Transition::IsStop:
         {
            break;
         }
      }
   }
   if(lastLumi.m_run != -1) {
      //end transitions
      for(int i = 0; i<iNStreams;++i) {
         returnValue.emplace_back(Phase::kEndLumi,lastLumi,i);
      }
      returnValue.emplace_back(Phase::kEndLumi,lastLumi,1000);
   }
   if(lastRun.m_run != -1) {
      //end transitions
      for(int i = 0; i<iNStreams;++i) {
         returnValue.emplace_back(Phase::kEndRun,lastRun,i);
      }
      returnValue.emplace_back(Phase::kEndRun,lastRun,1000);
   }
   return returnValue;
}

void test_config(std::vector<std::pair<Transition,Sync>> iTrans,int iNStreams) {
   auto expectedV = expectedValues(iTrans, iNStreams);
   {
      std::cout <<"=============test start==========="<<std::endl;
      EventProcessor ep(std::move(iTrans),iNStreams);
      FilesProcessor fp;
      fp.processFiles(ep);
   }
   std::vector<std::tuple<Phase,Sync,int>> orderedSeen;
   orderedSeen.reserve(s_seenSyncs.size());
   for(auto const& i: s_seenSyncs) {
//      std::cout <<i.first.m_run<<" "<<i.first.m_lumi<<" "<<i.first.m_event<<" "<<i.second<<std::endl;
      auto s = std::get<2>(i);
      if(std::get<1>(i).m_event > 0) {
         s=-2;
      }
      orderedSeen.emplace_back(std::get<0>(i),std::get<1>(i),s);
   }
   std::sort(orderedSeen.begin(),orderedSeen.end());

   auto orderedExpected = expectedV;
   std::sort(orderedExpected.begin(),orderedExpected.end());
/*   for(auto const& i: expectedV) {
      std::cout <<i.first.m_run<<" "<<i.first.m_lumi<<" "<<i.first.m_event<<" "<<i.second<<std::endl;      
   } */
   s_seenSyncs.clear();
   
   
   auto itOS = orderedSeen.begin();
   for(auto itOE = orderedExpected.begin(); itOE != orderedExpected.end(); ++itOE) {
      if(itOS == orderedSeen.end()) {
         break;
      }
      if ( *itOE != *itOS) {
         auto syncOE = std::get<1>(*itOE);
         auto syncOS = std::get<1>(*itOS);
         std::cout <<"Different ordering "<<syncOE.m_run<<" "<<syncOE.m_lumi<<" "<<syncOE.m_event<<" "<<std::get<2>(*itOE)<<"\n";
         std::cout <<"                   "<<syncOS.m_run<<" "<<syncOS.m_lumi<<" "<<syncOS.m_event<<" "<<std::get<2>(*itOS)<<"\n";
      }
      ++itOS;
   }

   if(orderedSeen.size() != orderedExpected.size()) {
      std::cout <<"Wrong number of transition "<<orderedSeen.size() <<" "<<orderedExpected.size()<<std::endl;
      return;
   }
   
}

int main() {
   tbb::task_scheduler_init scheduler{2};
   
  test_config( 
   {{Transition::IsFile,{0,0,0}}, 
    {Transition::IsRun,{1,0,0}}, 
    {Transition::IsLumi,{1,1,0}}, 
    {Transition::IsEvent,{1,1,1}},
    {Transition::IsEvent,{1,1,2}},
    {Transition::IsEvent,{1,1,3}},
    {Transition::IsEvent,{1,1,4}},
    {Transition::IsEvent,{1,1,5}},
    {Transition::IsStop,{0,0,0}}}, 2);

  //multiple different lumis
  test_config( 
   {{Transition::IsFile,{0,0,0}}, 
    {Transition::IsRun,{1,0,0}}, 
    {Transition::IsLumi,{1,1,0}}, 
    {Transition::IsEvent,{1,1,1}},
    {Transition::IsEvent,{1,1,2}},
    {Transition::IsLumi,{1,2,0}}, 
    {Transition::IsEvent,{1,2,3}},
    {Transition::IsEvent,{1,2,4}},
    {Transition::IsStop,{0,0,0}}}, 2);

  //empty lumi
  test_config( 
   {{Transition::IsFile,{0,0,0}}, 
    {Transition::IsRun,{1,0,0}}, 
    {Transition::IsLumi,{1,1,0}}, 
    {Transition::IsLumi,{1,2,0}}, 
    {Transition::IsEvent,{1,2,3}},
    {Transition::IsEvent,{1,2,4}},
    {Transition::IsStop,{0,0,0}}}, 2);

  //multiple different runs
  test_config( 
   {{Transition::IsFile,{0,0,0}}, 
    {Transition::IsRun,{1,0,0}}, 
    {Transition::IsLumi,{1,1,0}}, 
    {Transition::IsEvent,{1,1,1}},
    {Transition::IsEvent,{1,1,2}},
    {Transition::IsRun,{2,0,0}}, 
    {Transition::IsLumi,{2,1,0}}, 
    {Transition::IsEvent,{2,1,1}},
    {Transition::IsEvent,{2,1,2}},
    {Transition::IsStop,{0,0,0}}}, 2);

  //empty run
  test_config( 
   {{Transition::IsFile,{0,0,0}}, 
    {Transition::IsRun,{1,0,0}}, 
    {Transition::IsRun,{2,0,0}}, 
    {Transition::IsLumi,{2,1,0}}, 
    {Transition::IsEvent,{2,1,1}},
    {Transition::IsEvent,{2,1,2}},
    {Transition::IsStop,{0,0,0}}}, 2);

  //empty run
  test_config( 
   {{Transition::IsFile,{0,0,0}}, 
    {Transition::IsFile,{0,0,0}},
    {Transition::IsRun,{1,0,0}}, 
    {Transition::IsLumi,{1,1,0}}, 
    {Transition::IsEvent,{1,1,1}},
    {Transition::IsEvent,{1,1,2}}}, 2);

  //merging run across file boundary
  test_config( 
   {{Transition::IsFile,{0,0,0}}, 
    {Transition::IsRun,{1,0,0}}, 
    {Transition::IsLumi,{1,1,0}}, 
    {Transition::IsEvent,{1,1,1}},
    {Transition::IsEvent,{1,1,2}},
    {Transition::IsFile, {0,0,0}},
    {Transition::IsRun,{1,0,0}}, 
    {Transition::IsLumi,{1,2,0}}, 
    {Transition::IsEvent,{1,2,3}},
    {Transition::IsEvent,{1,2,4}},
    {Transition::IsStop,{0,0,0}}}, 2);

  //merging run & lumi across file boundary
  test_config( 
   {{Transition::IsFile,{0,0,0}}, 
    {Transition::IsRun,{1,0,0}}, 
    {Transition::IsLumi,{1,1,0}}, 
    {Transition::IsEvent,{1,1,1}},
    {Transition::IsEvent,{1,1,2}},
    {Transition::IsFile, {0,0,0}},
    {Transition::IsRun,{1,0,0}}, 
    {Transition::IsLumi,{1,1,0}}, 
    {Transition::IsEvent,{1,1,3}},
    {Transition::IsEvent,{1,1,4}},
    {Transition::IsStop,{0,0,0}}}, 2);

  //Files with delayed merge of lumis
  test_config( 
   {{Transition::IsFile,{0,0,0}}, 
    {Transition::IsRun,{1,0,0}}, 
    {Transition::IsLumi,{1,1,0}}, 
    {Transition::IsLumi,{1,1,0}}, //to merge
    {Transition::IsEvent,{1,1,1}},
    {Transition::IsEvent,{1,1,2}},
    {Transition::IsLumi,{1,2,0}}, 
    {Transition::IsEvent,{1,2,2}},
    {Transition::IsEvent,{1,2,3}},
    {Transition::IsStop,{0,0,0}}}, 2);
  
  //Files with delayed merge of runs
  test_config( 
   {{Transition::IsFile,{0,0,0}}, 
    {Transition::IsRun,{1,0,0}}, 
    {Transition::IsRun,{1,0,0}}, // to erge
    {Transition::IsLumi,{1,1,0}}, 
    {Transition::IsEvent,{1,1,1}},
    {Transition::IsEvent,{1,1,2}},
    {Transition::IsRun,{2,0,0}}, 
    {Transition::IsLumi,{2,1,0}}, 
    {Transition::IsEvent,{2,1,1}},
    {Transition::IsEvent,{2,1,2}},
    {Transition::IsStop,{0,0,0}}}, 2);
  
   
   return 0;
}

//g++ -std=c++14 -I/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/tbb/2018/include -I. concurrent_transitions.cc -L/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/tbb/2018/lib -L/root/buildArea/CMSSW_9_4_0_pre2/lib/slc6_amd64_gcc630/  -ltbb -lFWCoreConcurrency