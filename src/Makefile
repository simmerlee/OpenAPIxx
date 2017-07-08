target = libOpenAPIxx.a
objs = OA_Lock.o OA_Socket.o OA_Thread.o OA_Time.o OA_WorkerThread.o
cc = g++
cxxflags = -Wall -c -D OA_PLT_MACOSX=1

$(target) : $(objs)
	ar cvr $(target) $(objs)

OA_Lock.o : OA_Lock.cpp
	$(cc) $(cxxflags) $< -o $@
OA_Socket.o : OA_Socket.cpp
	$(cc) $(cxxflags) $< -o $@
OA_Thread.o : OA_Thread.cpp
	$(cc) $(cxxflags) $< -o $@
OA_Time.o : OA_Time.cpp
	$(cc) $(cxxflags) $< -o $@
OA_WorkerThread.o : OA_WorkerThread.cpp
	$(cc) $(cxxflags) $< -o $@

clean:
	rm -f $(target) $(objs)
