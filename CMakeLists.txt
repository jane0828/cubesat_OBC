cmake_minimum_required(VERSION 3.10)
project(OBC_Project C)

# 소스 파일 리스트
set(SOURCES
    main.c
    camsrc.c
    ledsrc.c
    functions.c
)

# 실행 파일 생성
add_executable(obc_app ${SOURCES})

# 필요시 라이브러리 링크 (예: socket, pthread)
target_link_libraries(obc_app pthread)
