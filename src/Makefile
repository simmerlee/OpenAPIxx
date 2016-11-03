target = libOpenAPI.a
objs = OA_Lock.o OA_Thread.o OA_Socket.o OA_Time.o
CC = g++
cxxflags = -W -c

$(target) : $(objs)
	ar -cvr $(target) $(objs)
OA_Lock.o : OA_Lock_linux.cpp OA_Lock.h
	$(CC) $(cxxflags) $< -o $@
OA_Thread.o : OA_Thread_linux.cpp OA_Thread.h
	$(CC) $(cxxflags) $< -o $@
OA_Socket.o : OA_Socket_linux.cpp OA_Socket.h
	$(CC) $(cxxflags) $< -o $@
OA_Time.o : OA_Time_linux.cpp OA_Time.h
	$(CC) $(cxxflags) $< -o $@

clean:
	rm -f $(target) $(objs)

