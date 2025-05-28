cmake_minimum_required(VERSION 3.28)

    project(processes)

        set(CMAKE_CXX_STANDARD 23)
        set(CMAKE_CXX_STANDARD_REQUIRED ON)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++23 -stdlib=libc++")
                set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

                    add_executable(${PROJECT_NAME}./ src / main.cpp./ src /
                                   worker.cpp./ src / helper.cpp./ src /
                                   semaphores.cpp./ src / pipes.cpp./ src /
                                   unit.cpp)
