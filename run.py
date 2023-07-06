import random
import os

BATCH_SIZE = 10
SNAP_VERTEX_NUM = 1 << 16
SNAP_EDGE_NUM = 16 * (1 << 16)
BASE_GRAPH_RATE = 0.5
BATCH_ADD_RATE = 0
BATCH_TIME = 1
DEGREE_AVG = 16

BATCH_SIZE_ARR = [10, 100, 1000, 10000, 100000, 500000, 1000000]
SNAP_VERTEX_NUM_ARR =  [1 << 16, 1 << 17, 1 << 18, 1 << 19, 1 << 20, 1 << 21]
DEGREE_AVG_ARR = range(8, 16)
BATCH_ADD_RATE_ARR = [0.5]

def makerun(BATCH_SIZE, SNAP_VERTEX_NUM, SNAP_EDGE_NUM, BATCH_ADD_RATE, BATCH_TIME, DEGREE_AVG) :
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

  if os.path.exists("/home/wangcx/tmp/notes_end.txt"):
    os.remove("/home/wangcx/tmp/notes_end.txt")
  else:
    print("The file does not exist")
  
  with open('/home/wangcx/tmp/notes_end.txt', 'a') as the_file:
    the_file.write("BATCH_SIZE = " + str(BATCH_SIZE) + "\n")
    the_file.write("SNAP_VERTEX_NUM = " + str(SNAP_VERTEX_NUM) + "\n")
    the_file.write("SNAP_EDGE_NUM = " + str(SNAP_EDGE_NUM) + "\n")
    the_file.write("BASE_GRAPH_RATE = " + str(BASE_GRAPH_RATE) + "\n")
    the_file.write("BATCH_ADD_RATE = " + str(BATCH_ADD_RATE) + "\n")
    the_file.write("BATCH_TIME = " + str(BATCH_TIME) + "\n")
    the_file.write("DEGREE_AVG = " + str(DEGREE_AVG) + "\n")
  
  cmd = ["make", "RunAll", "BATCH_SIZE=" + str(BATCH_SIZE), "SNAP_VERTEX_NUM=" + str(SNAP_VERTEX_NUM), "SNAP_EDGE_NUM=" + str(SNAP_EDGE_NUM), "BASE_GRAPH_RATE=" + str(BASE_GRAPH_RATE), "BATCH_ADD_RATE=" + str(BATCH_ADD_RATE), "BATCH_TIME="+str(BATCH_TIME), "DEGREE_AVG=" + str(DEGREE_AVG)]

  os.system(" ".join(cmd))

  return

# makerun(BATCH_SIZE, SNAP_VERTEX_NUM, SNAP_EDGE_NUM, BATCH_ADD_RATE, BATCH_TIME)

for n_1 in BATCH_SIZE_ARR :
  for n_2 in SNAP_VERTEX_NUM_ARR :
    for n_3 in BATCH_ADD_RATE_ARR :
      for n_4 in DEGREE_AVG_ARR :
        makerun(n_1, n_2, n_4 * n_2, n_3, BATCH_TIME, n_4)
        # exit(1)
