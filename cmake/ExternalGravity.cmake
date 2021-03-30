# Create download URL derived from version number.
set(Gravity_HOME https://github.com/coin-or/Gravity/archive)
set(Gravity_DOWNLOAD_URL ${Gravity_HOME}/matsup.tar.gz)
unset(Gravity_HOME)

# Download and build the Gravity library and add its properties to the third party arguments.
set(Gravity_ROOT_DIR ${THIRDPARTY_INSTALL_PATH}/Install/gravity CACHE INTERNAL "")
ExternalProject_Add(Gravity
    DOWNLOAD_DIR ${THIRDPARTY_INSTALL_PATH}
    DOWNLOAD_COMMAND export HTTP_PROXY=$ENV{HTTP_PROXY} && export HTTPS_PROXY=$ENV{HTTPS_PROXY} && export http_proxy=$ENV{HTTP_PROXY} && curl -k -L ${Gravity_DOWNLOAD_URL} -o Gravity-matsup.tar.gz && tar -xvf Gravity-matsup.tar.gz && rm -fr ./Install/gravity && mv Gravity-matsup Gravity && mv Gravity ./Install && cd ./Install/gravity && cmake . && make -j24 gravity
    URL ${Gravity_DOWNLOAD_URL}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)

list(APPEND GLOBAL_THIRDPARTY_LIB_ARGS "-DGravity_ROOT_DIR:PATH=${Gravity_ROOT_DIR}")
set(VORO_INCLUDE_DIRS ${Gravity_ROOT_DIR}/src)
include_directories(${Gravity_INCLUDE_DIRS})
find_library(Gravity_LIBRARY1
        libgravity.a
        HINTS /usr/local/lib
        HINTS ${Gravity_ROOT_DIR}/src
)


set(Gravity_LIBRARIES  ${Gravity_LIBRARY1})
set(LIBS ${LIBS} ${Gravity_LIBRARIES})
unset(Gravity_DOWNLOAD_URL)
unset(Gravity_ROOT)
