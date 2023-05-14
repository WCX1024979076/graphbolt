import random
import os

BATCH_SIZE = 100000
SNAP_VERTEX_NUM = 750000
SNAP_EDGE_NUM = 4194304
BASE_GRAPH_RATE = 0.5
BATCH_ADD_RATE = 0.7
BATCH_TIME = 1

BATCH_SIZE_ARR = [10, 100, 1000, 5000, 10000, 50000, 100000, 500000, 750000, 1000000, 2000000]
SNAP_VERTEX_NUM_ARR =  [10, 100, 1000, 5000, 10000, 50000, 100000, 500000, 750000, 1000000, 2000000]
SNAP_EDGE_NUM_ARR = [4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288, 1048576, 2097152, 4194304, 8388608]
BATCH_ADD_RATE_ARR = [0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0]

def makerun(BATCH_SIZE, SNAP_VERTEX_NUM, SNAP_EDGE_NUM, BATCH_ADD_RATE, BATCH_TIME) :
  if int(SNAP_EDGE_NUM * BASE_GRAPH_RATE) + int(BATCH_SIZE * BATCH_ADD_RATE) > SNAP_EDGE_NUM :
    return
  
  if int(SNAP_EDGE_NUM * BASE_GRAPH_RATE) < BATCH_SIZE - int(BATCH_SIZE * BATCH_ADD_RATE) :
    return
  
  if int(SNAP_EDGE_NUM * BASE_GRAPH_RATE) + (2 * int(BATCH_SIZE * BATCH_ADD_RATE) - BATCH_SIZE) * BATCH_TIME < 0:
    return 
  
  BATCH_RATE = BATCH_SIZE / (BASE_GRAPH_RATE * SNAP_EDGE_NUM)
  
  print("BATCH_SIZE =", BATCH_SIZE)
  print("SNAP_VERTEX_NUM =", SNAP_VERTEX_NUM)
  print("SNAP_EDGE_NUM =", SNAP_EDGE_NUM)
  print("BASE_GRAPH_RATE =", BASE_GRAPH_RATE)
  print("BATCH_ADD_RATE =", BATCH_ADD_RATE)

  if os.path.exists("/home/wangcx/tmp/notes.txt"):
    os.remove("/home/wangcx/tmp/notes.txt")
  else:
    print("The file does not exist")
  
  with open('/home/wangcx/tmp/notes.txt', 'a') as the_file:
    the_file.write("BATCH_SIZE = " + str(BATCH_SIZE) + "\n")
    the_file.write("SNAP_VERTEX_NUM = " + str(SNAP_VERTEX_NUM) + "\n")
    the_file.write("SNAP_EDGE_NUM = " + str(SNAP_EDGE_NUM) + "\n")
    the_file.write("BASE_GRAPH_RATE = " + str(BASE_GRAPH_RATE) + "\n")
    the_file.write("BATCH_ADD_RATE = " + str(BATCH_ADD_RATE) + "\n")
  
  cmd = ["make", "RunAll", "BATCH_SIZE=" + str(BATCH_SIZE), "SNAP_VERTEX_NUM=" + str(SNAP_VERTEX_NUM), "SNAP_EDGE_NUM=" + str(SNAP_EDGE_NUM), "BASE_GRAPH_RATE=" + str(BASE_GRAPH_RATE), "BATCH_ADD_RATE=" + str(BATCH_ADD_RATE), "BATCH_TIME="+str(BATCH_TIME)]

  os.system(" ".join(cmd))

  return

makerun(BATCH_SIZE, SNAP_VERTEX_NUM, SNAP_EDGE_NUM, BATCH_ADD_RATE, BATCH_TIME)

# for n in BATCH_SIZE_ARR:
#   makerun(n, SNAP_VERTEX_NUM, SNAP_EDGE_NUM, BATCH_ADD_RATE, BATCH_TIME)

# for n in SNAP_VERTEX_NUM_ARR:
#   makerun(BATCH_SIZE, n, SNAP_EDGE_NUM, BATCH_ADD_RATE, BATCH_TIME)

# for n in SNAP_EDGE_NUM_ARR:
#   makerun(BATCH_SIZE, SNAP_VERTEX_NUM, n, BATCH_ADD_RATE, BATCH_TIME)

# for n in BATCH_ADD_RATE_ARR:
#   makerun(BATCH_SIZE, SNAP_VERTEX_NUM, SNAP_EDGE_NUM, n, BATCH_TIME)
