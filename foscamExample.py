from foscam import FoscamCamera
from time import sleep

mycam = FoscamCamera('192.168.1.108', 88, 'admin', 'agua')
mycam.ptz_move_up()
sleep(1)
mycam.ptz_stop_run()