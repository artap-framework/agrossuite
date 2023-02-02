PROJECT(solver_plugin_UMFPACK C CXX)
CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake ${PROJECT_SOURCE_DIR}/../dealii/cmake)

SET(SOURCES
    main.cpp
    ../agros-library/solver/plugin_solver_interface.cpp)

IF(AGROS_UMFPACK)
    INCLUDE_DIRECTORIES(umfpack/include)
    INCLUDE_DIRECTORIES(umfpack/src)

    SET(SOURCE_UMFPACK
        umfpack/src/umf_assemble.cc
        umfpack/src/umf_blas3_update.cc
        umfpack/src/umf_build_tuples.cc
        umfpack/src/umf_create_element.cc
        umfpack/src/umf_dump.cc
        umfpack/src/umf_extend_front.cc
        umfpack/src/umf_garbage_collection.cc
        umfpack/src/umf_get_memory.cc
        umfpack/src/umf_init_front.cc
        umfpack/src/umf_kernel.cc
        umfpack/src/umf_kernel_init.cc
        umfpack/src/umf_kernel_wrapup.cc
        umfpack/src/umf_local_search.cc
        umfpack/src/umf_lsolve.cc
        umfpack/src/umf_ltsolve.cc
        umfpack/src/umf_mem_alloc_element.cc
        umfpack/src/umf_mem_alloc_head_block.cc
        umfpack/src/umf_mem_alloc_tail_block.cc
        umfpack/src/umf_mem_free_tail_block.cc
        umfpack/src/umf_mem_init_memoryspace.cc
        umfpack/src/umf_report_vector.cc
        umfpack/src/umf_row_search.cc
        umfpack/src/umf_scale_column.cc
        umfpack/src/umf_set_stats.cc
        umfpack/src/umf_solve.cc
        umfpack/src/umf_symbolic_usage.cc
        umfpack/src/umf_transpose.cc
        umfpack/src/umf_tuple_lengths.cc
        umfpack/src/umf_usolve.cc
        umfpack/src/umf_utsolve.cc
        umfpack/src/umf_valid_numeric.cc
        umfpack/src/umf_valid_symbolic.cc
        umfpack/src/umf_grow_front.cc
        umfpack/src/umf_start_front.cc
        umfpack/src/umf_2by2.cc
        umfpack/src/umf_store_lu.cc
        umfpack/src/umf_scale.cc
        umfpack/src/umfpack_col_to_triplet.cc
        umfpack/src/umfpack_defaults.cc
        umfpack/src/umfpack_free_numeric.cc
        umfpack/src/umfpack_free_symbolic.cc
        umfpack/src/umfpack_get_numeric.cc
        umfpack/src/umfpack_get_lunz.cc
        umfpack/src/umfpack_get_symbolic.cc
        umfpack/src/umfpack_get_determinant.cc
        umfpack/src/umfpack_numeric.cc
        umfpack/src/umfpack_qsymbolic.cc
        umfpack/src/umfpack_report_control.cc
        umfpack/src/umfpack_report_info.cc
        umfpack/src/umfpack_report_matrix.cc
        umfpack/src/umfpack_report_numeric.cc
        umfpack/src/umfpack_report_perm.cc
        umfpack/src/umfpack_report_status.cc
        umfpack/src/umfpack_report_symbolic.cc
        umfpack/src/umfpack_report_triplet.cc
        umfpack/src/umfpack_report_vector.cc
        umfpack/src/umfpack_solve.cc
        umfpack/src/umfpack_symbolic.cc
        umfpack/src/umfpack_transpose.cc
        umfpack/src/umfpack_triplet_to_col.cc
        umfpack/src/umfpack_scale.cc
        umfpack/src/umfpack_load_numeric.cc
        umfpack/src/umfpack_save_numeric.cc
        umfpack/src/umfpack_load_symbolic.cc
        umfpack/src/umfpack_save_symbolic.cc
        umfpack/src/umfpack_global.cc
        umfpack/src/umfpack_tictoc.cc
        umfpack/src/umfpack_timer.cc
        umfpack/src/umf_analyze.cc
        umfpack/src/umf_apply_order.cc
        umfpack/src/umf_colamd.cc
        umfpack/src/umf_free.cc
        umfpack/src/umf_fsize.cc
        umfpack/src/umf_is_permutation.cc
        umfpack/src/umf_malloc.cc
        umfpack/src/umf_realloc.cc
        umfpack/src/umf_report_perm.cc
        umfpack/src/umf_singletons.cc
        umfpack/src/umf_triplet.cc
        umfpack/src/umf_multicompile.cc
        umfpack/src/amd_1.cc
        umfpack/src/amd_2.cc
        umfpack/src/amd_aat.cc
        umfpack/src/amd_control.cc
        umfpack/src/amd_defaults.cc
        umfpack/src/amd_dump.cc
        umfpack/src/amd_info.cc
        umfpack/src/amd_order.cc
        umfpack/src/amd_postorder.cc
        umfpack/src/amd_post_tree.cc
        umfpack/src/amd_preprocess.cc
        umfpack/src/amd_valid.cc
        umfpack/src/amd_global.cc)
ELSE(AGROS_UMFPACK)
    FIND_PACKAGE(LAPACK REQUIRED)
    FIND_PACKAGE(UMFPACK REQUIRED)

    INCLUDE_DIRECTORIES(${UMFPACK_INCLUDE_DIR})
ENDIF(AGROS_UMFPACK)

INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../3rdparty/")

FIND_PACKAGE(deal.II HINTS "${CMAKE_SOURCE_DIR}/../dealii/build" REQUIRED)

# Handle Qt.
SET(CMAKE_AUTOMOC TRUE)

FIND_PACKAGE(Qt6Core REQUIRED)

ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_SHARED)
ADD_DEFINITIONS(-DQT_DLL)
IF(AGROS_UMFPACK)
    ADD_LIBRARY(${PROJECT_NAME} SHARED ${SOURCES} ${SOURCE_UMFPACK})
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} Qt6::Core)
ELSE(AGROS_UMFPACK)
    ADD_LIBRARY(${PROJECT_NAME} SHARED ${SOURCES})
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${DEAL_II_LIBRARIES} ${LAPACK_LIBRARIES} ${UMFPACK_LIBRARIES} Qt6::Core)
ENDIF(AGROS_UMFPACK)
