
#[Unit]
#Description=SERVICE_NAME
#After=network.target
#StartLimitIntervalSec=0
#[Service]
#Type=simple
#Restart=always
#RestartSec=1
#User=centos
#ExecStart=CURRENT_DIR/Recorder
#
#[Install]
#WantedBy=multi-user.target

# ask what the user wants to name the service

echo "What would you like to name the service?"
read -r SERVICE_NAME

# get the current directory
CURRENT_DIR=$(pwd)

# get the user
USER=$(whoami)

# create the service file
echo "[Unit]
Description=$SERVICE_NAME
After=network.target
StartLimitIntervalSec=0
[Service]
Type=simple
Restart=always
RestartSec=1
User=$USER
ExecStart=$CURRENT_DIR/Recorder
[Install]
WantedBy=multi-user.target" > /etc/systemd/system/$SERVICE_NAME.service

# reload the daemon
systemctl daemon-reload

# enable the service
systemctl enable $SERVICE_NAME

# start the service
systemctl start $SERVICE_NAME

# check the status of the service
systemctl status $SERVICE_NAME
```
