sleep 3

sudo ps aux | grep GoQat | grep -v grep | awk {'print $2'} | xargs sudo kill
sudo /home/linaro/Rlags_project/scripts/sedi_camera/start_goqat.sh &

sudo /home/linaro/Rlags_project/scripts/drives/mount_drives
sudo rm -f /root/GoQat/ccd_display.fit
