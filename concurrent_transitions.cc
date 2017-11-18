#include <map>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <thread>
#include <mutex>
#include "tbb/task_scheduler_init.h"
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

static std::mutex s_logMutex;

static std::map<Transition,std::string> s_transToName = {
  {Transition::IsStop, "Stop"},
  {Transition::IsFile, "File"},
  {Transition::IsRun, "Run"},
  {Transition::IsLumi, "Lumi"},
  {Transition::IsEvent, "Event"} 
};

void printTrans( Transition iT) {
   std::lock_guard<std::mutex> g{s_logMutex};
   std::cout <<"Source: "<<s_transToName[iT]<<std::endl;
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
     
     void processOneEventAsync(edm::WaitingTaskHolder iTask) {
        dummyWorkAsync(std::move(iTask),"Event",Sync{0,0,0});
     }
     void processOneBeginLumiAsync(edm::WaitingTaskHolder iTask, const Sync& iLumiID) {
        dummyWorkAsync(std::move(iTask),"beginLumi",iLumiID);
     }
     void processOneBeginRunAsync(edm::WaitingTaskHolder iTask, int iRun) {
        dummyWorkAsync(std::move(iTask),"beginRun",Sync{iRun,0,0});        
     }

     void processOneEndLumiAsync(edm::WaitingTaskHolder iTask, const Sync& iLumiID) {
        dummyWorkAsync(std::move(iTask),"endLumi",iLumiID);
     }
     void processOneEndRunAsync(edm::WaitingTaskHolder iTask, int iRun) {
        dummyWorkAsync(std::move(iTask),"endRun",Sync{iRun,0,0});        
     }
     
  private:
     void dummyWorkAsync(edm::WaitingTaskHolder iTask, const char* iTran, const Sync& iSync) {
        auto streamID = m_streamID;
        
        auto t = edm::make_functor_task(tbb::task::allocate_root(), [iTask,iTran,streamID,iSync]() mutable {
             using namespace std::chrono_literals;
             {
                std::lock_guard<std::mutex> g{s_logMutex};
                std::cout <<"Stream transition "<<iSync.m_run<<" "<<iSync.m_lumi<<" "<<iTran<<" stream:"<<streamID<<std::endl;
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
     void processOneBeginLumiAsync(edm::WaitingTaskHolder iTask,Sync const& iSync) {
        dummyWorkAsync(std::move(iTask), [iSync]() { 
           std::lock_guard<std::mutex> g{s_logMutex};
           std::cout <<"Begin Global Lumi "<<iSync.m_run<<" "<<iSync.m_lumi<<std::endl;
           });
     }
     void processOneBeginRunAsync(edm::WaitingTaskHolder iTask, int iRun) {
        dummyWorkAsync(std::move(iTask), 
        [iRun]() { 
             std::lock_guard<std::mutex> g{s_logMutex};
             std::cout <<"Begin Global Run "<<iRun<<std::endl;
             });
     }

     void processOneEndLumiAsync(edm::WaitingTaskHolder iTask,Sync const& iSync) {
        dummyWorkAsync(std::move(iTask), [iSync]() { 
           std::lock_guard<std::mutex> g{s_logMutex};
           std::cout <<"End Global Lumi "<<iSync.m_run<<" "<<iSync.m_lumi<<std::endl;
           });
     }
     void processOneEndRunAsync(edm::WaitingTaskHolder iTask, int iRun) {
        dummyWorkAsync(std::move(iTask), 
        [iRun]() { 
             std::lock_guard<std::mutex> g{s_logMutex};
             std::cout <<"End Global Run "<<iRun<<std::endl;
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

class EventProcessor {
public:
   EventProcessor(std::vector<std::pair<Transition,Sync>> iTransitions,
   int iNStreams):
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
   
   Sync sync() const { return m_source.sync();}
   
   void closeInputFile() {}
   void closeOutputFiles() {}

   void readRun() {}
   void beginRun(int iRunNumber) {
      {
         std::lock_guard<std::mutex> g{s_logMutex};
         std::cout <<" beginRun "<<iRunNumber<<std::endl;      
      }
      auto globalWaitTask = edm::make_empty_waiting_task();
      auto globalWaitTaskPtr = globalWaitTask.get();
      globalWaitTask->increment_ref_count();
      m_globalSchedule.processOneBeginRunAsync(edm::WaitingTaskHolder{globalWaitTask.get()},iRunNumber);
      globalWaitTask->wait_for_all();
      
      {
         auto streamWaitTask = edm::make_empty_waiting_task();
         auto streamWaitTaskPtr = globalWaitTask.get();
         streamWaitTask->increment_ref_count();
         
         {
            edm::WaitingTaskHolder holdUntilAllStreamsCalled{streamWaitTask.get()};
            for(unsigned int i=0; i<m_nStreams;++i) {
               m_streamSchedules[i].processOneBeginRunAsync(edm::WaitingTaskHolder(holdUntilAllStreamsCalled), iRunNumber);
            }
         }
         streamWaitTask->wait_for_all();
      }
   }
   void readAndMergeRun() {
      std::lock_guard<std::mutex> g{s_logMutex};
      std::cout <<" merge run "<<std::endl; //<<runID.m_run<<std::endl;
   }
      
   void endRun(int iRunNumber) {
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
               m_streamSchedules[i].processOneEndRunAsync(edm::WaitingTaskHolder(holdUntilAllStreamsCalled), iRunNumber);
            }
         }
         streamWaitTask->wait_for_all();
      }
      auto globalWaitTask = edm::make_empty_waiting_task();
      auto globalWaitTaskPtr = globalWaitTask.get();
      globalWaitTask->increment_ref_count();
      m_globalSchedule.processOneEndRunAsync(edm::WaitingTaskHolder{globalWaitTask.get()},iRunNumber);
      globalWaitTask->wait_for_all();      
   }
   void writeRun(int iRunNumber) {}
   void deleteRunFromCache(int iRunNumber) {}

   void readLuminosityBlock() {}
   void beginLumi(Sync const& iSync) {
      {
         std::lock_guard<std::mutex> g{s_logMutex};
         std::cout <<"beginLumi "<< iSync.m_run<<" "<<iSync.m_lumi<<std::endl;
      }
      auto globalWaitTask = edm::make_empty_waiting_task();
      auto globalWaitTaskPtr = globalWaitTask.get();
      globalWaitTask->increment_ref_count();
      m_globalSchedule.processOneBeginLumiAsync(edm::WaitingTaskHolder{globalWaitTask.get()},iSync);
      globalWaitTask->wait_for_all();
      
      {
         auto streamWaitTask = edm::make_empty_waiting_task();
         auto streamWaitTaskPtr = streamWaitTask.get();
         streamWaitTask->increment_ref_count();
         
         {
            edm::WaitingTaskHolder holdUntilAllStreamsCalled{streamWaitTask.get()};
            for(unsigned int i=0; i<m_nStreams;++i) {
               m_streamSchedules[i].processOneBeginLumiAsync(edm::WaitingTaskHolder(holdUntilAllStreamsCalled),iSync);
            }
         }
         streamWaitTask->wait_for_all();
      }
      
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
      
      {
         auto streamWaitTask = edm::make_empty_waiting_task();
         auto streamWaitTaskPtr = streamWaitTask.get();
         streamWaitTask->increment_ref_count();
         
         {
            edm::WaitingTaskHolder holdUntilAllStreamsCalled{streamWaitTask.get()};
            for(unsigned int i=0; i<m_nStreams;++i) {
               m_streamSchedules[i].processOneEndLumiAsync(edm::WaitingTaskHolder(holdUntilAllStreamsCalled),iSync);
            }
         }
         streamWaitTask->wait_for_all();
      }
      {
         auto globalWaitTask = edm::make_empty_waiting_task();
         auto globalWaitTaskPtr = globalWaitTask.get();
         globalWaitTask->increment_ref_count();
         m_globalSchedule.processOneEndLumiAsync(edm::WaitingTaskHolder{globalWaitTask.get()},iSync);
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
    //To wait, the ref count has to b 1+#streams
    auto eventLoopWaitTask = edm::make_empty_waiting_task();
    auto eventLoopWaitTaskPtr = eventLoopWaitTask.get();
    eventLoopWaitTask->increment_ref_count();

    unsigned int iStreamIndex = 0;
    for(; iStreamIndex<m_nStreams-1; ++iStreamIndex) {
      eventLoopWaitTask->increment_ref_count();
      tbb::task::enqueue( *edm::make_waiting_task(tbb::task::allocate_root(),[this,iStreamIndex,eventLoopWaitTaskPtr](std::exception_ptr const*){
        handleNextEventForStreamAsync(eventLoopWaitTaskPtr,iStreamIndex);
      }) );
    }
    eventLoopWaitTask->increment_ref_count();
    eventLoopWaitTask->spawn_and_wait_for_all( *edm::make_waiting_task(tbb::task::allocate_root(),[this,iStreamIndex,eventLoopWaitTaskPtr](std::exception_ptr const*){
      handleNextEventForStreamAsync(eventLoopWaitTaskPtr,iStreamIndex);
    }));

    return nextItemTypeFromProcessingEvents_.load();
      
   }
   
private:
   
   void handleNextEventForStreamAsync(edm::WaitingTask* iTask, unsigned int iStreamIndex) {
      auto recursionTask = edm::make_waiting_task(tbb::task::allocate_root(), [this,iTask,iStreamIndex](std::exception_ptr const* iPtr) {
        if(iPtr) {
           edm::WaitingTaskHolder h(iTask);
           h.doneWaiting(*iPtr);
          //the stream will stop now
          iTask->decrement_ref_count();
          return;
        }

        handleNextEventForStreamAsync(iTask, iStreamIndex);
      });

      m_sourceQueue.push([this,recursionTask,iTask,iStreamIndex]() {

             try {
               if(readNextEventForStream(iStreamIndex) ) {
                 processEventAsync( edm::WaitingTaskHolder(recursionTask), iStreamIndex);
               } else {
                 //the stream will stop now
                 tbb::task::destroy(*recursionTask);
                 iTask->decrement_ref_count();
               }
             } catch(...) {
               edm::WaitingTaskHolder h(recursionTask);
               h.doneWaiting(std::current_exception());
             }
      });
      
   }
   bool readNextEventForStream(unsigned int iStreamIndex) {
      if(not m_firstEventInBlock) {
         //The state machine already called input_->nextItemType
         // and found an event. We can't call input_->nextItemType
         // again since it would move to the next transition
         Transition itemType = m_source.nextItemType();
         if (Transition::IsEvent !=itemType) {
           nextItemTypeFromProcessingEvents_ = itemType;
           //std::cerr<<"next item type "<<itemType<<"\n";
           return false;
        }
      } else {
         m_firstEventInBlock=false;
      }
      return true;
   }
   
   void processEventAsync(edm::WaitingTaskHolder iHolder,
                          unsigned int iStreamIndex) {
     tbb::task::spawn( *edm::make_functor_task( tbb::task::allocate_root(), [=]() {
       processEventAsyncImpl(iHolder, iStreamIndex);
     }) );
   }
   
   void processEventAsyncImpl(edm::WaitingTaskHolder iHolder,
      unsigned int iStreamIndex) {
         m_streamSchedules[iStreamIndex].processOneEventAsync(std::move(iHolder));
      }
   
   
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
               processLumi(iEP, iRun);
               nextTrans = iEP.nextTransitionType();
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
   
   void processLumi(EventProcessor& iEP, std::shared_ptr<RunResources> iRun) {
      auto lumiID =iEP.sync();
      if( (not m_currentLumi) or 
          m_currentLumi->m_run.get() != iRun.get() or 
          m_currentLumi->lumiID() != lumiID)
      {
         //switch to different lumi
         m_currentLumi = std::make_shared<LumiResources>(std::move(iRun), lumiID.m_lumi );
         iEP.readLuminosityBlock();
         iEP.beginLumi(lumiID);
         
      } else {
         iEP.readAndMergeLumi();
      } 
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
         iEP.beginRun(runID.m_run);         
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


int main() {
   tbb::task_scheduler_init scheduler{2};
   
   EventProcessor ep( {{Transition::IsFile,{0,0,0}}, 
   {Transition::IsRun,{1,0,0}}, 
   {Transition::IsLumi,{1,1,0}}, 
   {Transition::IsEvent,{1,1,1}},
   {Transition::IsEvent,{1,1,2}},
   {Transition::IsEvent,{1,1,3}},
   {Transition::IsEvent,{1,1,4}},
   {Transition::IsStop,{0,0,0}}}, 2);
   
   bool contLoop = true;
   FilesProcessor fp;
   fp.processFiles(ep);
   
   return 0;
}

//g++ -std=c++14 -I/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/tbb/2018/include -I. concurrent_transitions.cc -L/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/tbb/2018/lib -L/root/buildArea/CMSSW_9_4_0_pre2/lib/slc6_amd64_gcc630/  -ltbb -lFWCoreConcurrency