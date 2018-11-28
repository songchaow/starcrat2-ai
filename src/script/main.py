# python 3
# configure LD_LIBRARY_PATH for `micromachine` library
#import os, sys
#os.environ['LD_LIBRARY_PATH'] = os.path.dirname(os.getcwd())+"/lib:"+os.environ['LD_LIBRARY_PATH']
#try:
#    os.execv(sys.executable, ['python3.6'] + sys.argv)
#except Exception as e:
#    print('Re-execute failed.')
#    print(str(e))

# --------------------------
import micromachine
import sys
mm_obj = micromachine.MicroMachine()
mm_obj.Initialize(sys.argv)
#construct a RegionMoveCommand object
first_command = micromachine.RegionMoveCommand(
        micromachine.RegionID.REGION_A,micromachine.RegionID.REGION_B)

mm_obj.AddRegionMoveCommand(first_command) 

i = 0
while(True):
    i = i + 1
    mm_obj.Update()
    print("Frame: "+str(i))
    command = micromachine.RegionMoveCommand(
        micromachine.RegionID.REGION_A,micromachine.RegionID.REGION_B)

    mm_obj.AddRegionMoveCommand(command)
pass
