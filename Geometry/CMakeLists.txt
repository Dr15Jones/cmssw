add_subdirectory(CaloGeometry)
add_subdirectory(CommonTopologies)

add_custom_target(Geometry_all)

add_dependencies(Geometry_all
  GeometryCaloGeometry
  GeometryCommonTopologies
)
