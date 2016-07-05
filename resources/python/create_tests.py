#!/usr/bin/python3

import tests

def create_ctest(file = ""):
    if (file == ""):
        file = "CMakeTests.cmake"
        
    with open(file, "w") as text_file: 
        for g in tests.suite.all_tests():
            print('IF(TEST_' + g + ')', file=text_file)
            #for t in __tests__[g]:
            #    
            #    print('    agros_test("' + test_name + '")', file=text_file)
            #print('ENDIF(TEST_' + g + ')\n', file=text_file)
            for t in tests.suite.test(g):
                test_name = t.__module__ + "." + t.__name__
                print('    add_test(NAME ' + test_name + ' WORKING_DIRECTORY . COMMAND python3 -m unittest ' + test_name + ')', file=text_file)
            print('ENDIF(TEST_' + g + ')\n', file=text_file)

if __name__ == '__main__':
    create_ctest()