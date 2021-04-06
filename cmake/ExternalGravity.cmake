# Create download URL derived from version number.
set(Gravity_HOME https://github.com/coin-or/Gravity/archive)
set(Gravity_DOWNLOAD_URL ${Gravity_HOME}/matsup.tar.gz)
unset(Gravity_HOME)

# Download and build the Gravity library and add its properties to the third party arguments.
set(Gravity_ROOT_DIR ${THIRDPARTY_INSTALL_PATH}/Install/gravity CACHE INTERNAL "")
ExternalProject_Add(gravity
    DOWNLOAD_DIR ${THIRDPARTY_INSTALL_PATH}
    DOWNLOAD_COMMAND export HTTP_PROXY=$ENV{HTTP_PROXY} && export HTTPS_PROXY=$ENV{HTTPS_PROXY} && export http_proxy=$ENV{HTTP_PROXY} && curl -k -L ${Gravity_DOWNLOAD_URL} -o Gravity-matsup.tar.gz && tar -xvf Gravity-matsup.tar.gz && rm -fr ./Install/gravity && mv Gravity-matsup gravity && mv gravity ./Install && cd ./Install/gravity && cmake . && make -j24 gravity
    URL ${Gravity_DOWNLOAD_URL}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)

list(APPEND GLOBAL_THIRDPARTY_LIB_ARGS "-DGravity_ROOT_DIR:PATH=${Gravity_ROOT_DIR}")
set(GRAVITY_INCLUDE_DIRS ${Gravity_ROOT_DIR}/include)
include_directories(${GRAVITY_INCLUDE_DIRS})
find_library(GRAVITY_LIBRARY1
        libgravity.a
        HINTS /usr/local/lib
        HINTS ${Gravity_ROOT_DIR}/lib
)


set(GRAVITY_LIBRARIES  ${GRAVITY_LIBRARY1})
set(LIBS ${LIBS} ${GRAVITY_LIBRARIES})
unset(Gravity_DOWNLOAD_URL)
unset(Gravity_ROOT)
