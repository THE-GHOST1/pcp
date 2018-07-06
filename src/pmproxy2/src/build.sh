#!/bin/bash
#go to top dir (pcp)
cd /home/prajwal/CLionProjects/pcp
#remove all so files
sudo make clean
#git remote add upstream https://github.com/performancecopilot/pcp.git
#git fetch upstream
#git checkout master
#git merge upstream/master
./configure --prefix=/usr --libexecdir=/usr/lib --sysconfdir=/etc --localstatedir=/var --with-rcdir=/etc/init.d --with-webapi
sudo make
sudo make install
find /usr/lib/ -path '*pcp*.so*'
sudo updatedb
sudo ldconfig
#make ln -s
#make clean
