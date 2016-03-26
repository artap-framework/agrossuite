PROJECT(${NSGA3_LIBRARY})

SET(HEADERS alg_comparator.h  alg_crossover.h  alg_environmental_selection.h  alg_individual.h  alg_initialization.h  alg_mutation.h  alg_nondominated_sort.h  alg_nsgaiii.h  alg_population.h  alg_reference_point.h aux_math.h aux_misc.h problem_base.h)
SET(SOURCES alg_comparator.cpp  alg_crossover.cpp  alg_environmental_selection.cpp  alg_individual.cpp  alg_initialization.cpp  alg_mutation.cpp  alg_nondominated_sort.cpp  alg_nsgaiii.cpp  alg_reference_point.cpp aux_math.cpp aux_misc.cpp)

ADD_LIBRARY(${PROJECT_NAME} ${HEADERS} ${SOURCES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME})
