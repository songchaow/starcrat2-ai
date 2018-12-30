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
from s2clientprotocol import sc2api_pb2
import units

mm_obj = micromachine.MicroMachine()

path_arg = ["-e","/home/aidev/packages/StarCraftII_game/Versions/Base60321/SC2_x64"]
# mm_obj.Initialize(sys.argv)
mm_obj.Initialize(path_arg)
#construct a RegionMoveCommand object
first_command = micromachine.RegionMoveCommand(
        micromachine.RegionID.REGION_A,micromachine.RegionID.REGION_B)

mm_obj.AddRegionMoveCommand(first_command) 

i = 0

mm_obj.Update()
buff = mm_obj.GetSerializedObservation()
obs = sc2api_pb2.Observation()
# obs.ParseFromString(buff)
# print(str(obs))
ended = False
res = micromachine.GameResult.Undecided
while(not ended):
   # Let's create some soldiers
   create_cmd = micromachine.CreateCommand(units.Terran.SiegeTank)
   mm_obj.AddProductionCommand(create_cmd)
   result = create_cmd.GetResult()
   if(result):
       if(result.m_result):
           print("Create Siege Tank succeeded.")
       else:
           print("Create Siege Tank failed. Need...")
   else:
       print("No result generated.")
   
   i = i + 1
   ended, res = mm_obj.Update()
   print("Frame: "+str(i))
   command = micromachine.RegionMoveCommand(
       micromachine.RegionID.REGION_A,micromachine.RegionID.REGION_B)
   mm_obj.AddRegionMoveCommand(command)

print("Result: "+str(res))
pass
