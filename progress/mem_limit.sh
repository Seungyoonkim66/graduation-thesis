#!/bin/bash
  
export CGROUP_NAME=seungyoon
export CGROUP_DIR=/sys/fs/cgroup

if [ "x$1" == "xremove" ]; then
        sudo rmdir ${CGROUP_DIR}/${CGROUP_NAME}
        exit
fi

if [ ! -d "${CGROUP_DIR}/${CGROUP_NAME}" ]; then
        sudo mkdir -p ${CGROUP_DIR}/${CGROUP_NAME}
fi

echo "+memory" | sudo tee ${CGROUP_DIR}/cgroup.subtree_control



if [[ "$1" =~ "MB" ]] || [[ "$1" =~ "GB" ]]; then
        MEM_SIZE=$1

MEM_SIZE=${MEM_SIZE^^}

if [[ ${MEM_SIZE} =~ "MB" ]]; then
        MEM_IN_BYTES=`echo ${MEM_SIZE::-2}*1024*1024 | bc`
elif [[ ${MEM_SIZE} =~ "GB" ]]; then
        MEM_IN_BYTES=`echo ${MEM_SIZE::-2}*1024*1024*1024 | bc`
fi

echo ${MEM_IN_BYTES} | sudo tee /${CGROUP_DIR}/${CGROUP_NAME}/memory.max

fi

#KSWAPD_WAKEUP_LIMIT=`echo ${MEM_IN_BYTES}*99/100 | bc`
#launch app in this cgroup
# sh -c "echo \$$ | sudo tee /${CGROUP_DIR}/${CGROUP_NAME}/cgroup.procs && ls"
