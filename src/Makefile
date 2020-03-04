CC = gcc
CXXFLAGS = -W -c -I $(INC_DIR) -D $(PLATFORM)=1
TARGET = libOpenAPIxx.a
OBJS = OA_Lock.o OA_Socket.o OA_Thread.o OA_Time.o OA_WorkerThread.o \
       OA_ArgParser.o OA_ASCII.o

all:
	cd $(OUT_DIR) && $(CC) $(CXXFLAGS) $(SRC_DIR)/*.cpp
	cd $(OUT_DIR) && ar cvr $(TARGET) $(OBJS)


