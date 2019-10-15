include(FindGit)
 
set (GIT_BRANCH "")
set  (GIT_COMMIT_HASH "")

 message(STATUS "-- Extracting short hash from git... ")
  # Get the current working branch
  execute_process(
   COMMAND ${GIT_EXECUTABLE} name-rev --name-only HEAD
   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
   OUTPUT_VARIABLE GIT_BRANCH0
   OUTPUT_STRIP_TRAILING_WHITESPACE
  )
 
  # Clean Branch name to have only the branch
  STRING(REGEX REPLACE "((origin))+" "" GIT_BRANCH1 ${GIT_BRANCH0})
  STRING(REGEX REPLACE "((remotes))+" "" GIT_BRANCH2 ${GIT_BRANCH1})
  STRING(REGEX REPLACE "\\/+" "" GIT_BRANCH3 ${GIT_BRANCH2})
  STRING(REGEX REPLACE "~[0-9]*" "" GIT_BRANCH4 ${GIT_BRANCH3})
  STRING(REGEX REPLACE "\\^[0-9]*" "" GIT_BRANCH ${GIT_BRANCH4})

  # Get the latest abbreviated commit hash of the working branch
  execute_process(
  COMMAND git log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  string(TIMESTAMP CURRENT_YEAR "%Y")
 
 set (DNAME_ORGANIZATION "Moonlight") 
message(STATUS "${CMAKE_PROJECT_NAME} version " "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}  - ${GIT_BRANCH4}- ${GIT_COMMIT_HASH}.")
configure_file("./src/configuration.h.in" "${PROJECT_BINARY_DIR}/configuration.h")
