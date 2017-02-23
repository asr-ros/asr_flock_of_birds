cmake_minimum_required(VERSION 2.8.3)
project(asr_flock_of_birds)


## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
find_package(catkin REQUIRED COMPONENTS
	roscpp
	tf
	geometry_msgs
	std_msgs
	std_srvs
	asr_msgs
)

find_package(Boost REQUIRED COMPONENTS 
	system
	program_options
)

catkin_package(
	CATKIN_DEPENDS	roscpp tf geometry_msgs	std_msgs std_srvs asr_msgs
	DEPENDS	Boost
)

include_directories(
	${PROJECT_SOURCE_DIR}/include
	${catkin_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
)

add_executable(flock_of_birds_server 
	src/bird_track_impl.cc 
	src/main.cc  
	src/tracker_impl.cc  
	src/transform_coords.cc 
	src/serial.cc)

# dependcy to the message generation of asr_msgs to ensure 
# the message header files exist before trying to compile this package
add_dependencies(flock_of_birds_server asr_msgs_generate_messages_cpp)
	
target_link_libraries(flock_of_birds_server	 
	${catkin_LIBRARIES}
	${Boost_LIBRARIES}
)

