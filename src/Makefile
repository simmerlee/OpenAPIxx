target = libOpenAPIxx.a
cc = g++
cxxflags = -W -c $(debug_flag)
objs = OA_Lock.o OA_Socket.o OA_Thread.o OA_Time.o OA_WorkerThread.o

$(target) : $(objs)
	ar cvr $(out_dir)/$(target) $(objs)
OA_Lock.o : OA_Lock_linux.cpp
	$(cc) $(cxxflags) $< -o $@
OA_Socket.o : OA_Lock_linux.cpp
	$(cc) $(cxxflags) $< -o $@
OA_Thread.o : OA_Thread_linux.cpp
	$(cc) $(cxxflags) $< -o $@
OA_Time.o : OA_Time_linux.cpp
	$(cc) $(cxxflags) $< -o $@
OA_WorkerThread.o : OA_WorkerThread.cpp
	$(cc) $(cxxflags) $< -o $@

clean :
	rm -f $(objs)

