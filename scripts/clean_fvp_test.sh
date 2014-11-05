DEV_PATH=$HOME/devel/fvp_optee
DST_KERNEL=$DEV_PATH/linux
AARCH64_NONE_GCC=aarch64-none-elf
DST_AARCH64_NONE_GCC=$DEV_PATH/toolchains/$AARCH64_NONE_GCC
DST_GEN_ROOTFS=$DEV_PATH/gen_rootfs
AARCH64_GCC=aarch64
DST_AARCH64_GCC=$DEV_PATH/toolchains/$AARCH64_GCC
KERNEL_VERSION=`cd $DST_KERNEL && make kernelversion`
DST_OPTEE_CLIENT=$DEV_PATH/optee_client
DST_EDK2=$DEV_PATH/edk2
DST_ATF=$DEV_PATH/arm-trusted-firmware
DST_OPTEE_OS=$DEV_PATH/optee_os
DST_OPTEE_LK=$DEV_PATH/optee_linuxdriver
#clean linuxkernel
export PATH=$DST_AARCH64_NONE_GCC/bin:$PATH
export CROSS_COMPILE=$DST_AARCH64_NONE_GCC/bin/aarch64-none-elf-
cd  $DST_KERNEL
if [ ! -f ".config" ]; then
        make ARCH=arm64 defconfig
fi

make  LOCALVERSION= ARCH=arm64 clean

#########clean op_tee os
export PATH=$DST_AARCH32_GCC/bin:$PATH
export CROSS_COMPILE=arm-linux-gnueabihf-
export PLATFORM=vexpress
export PLATFORM_FLAVOR=fvp
export O=./out-os-fvp
export CFG_TEE_CORE_LOG_LEVEL=5
#export DEBUG=1

cd $DST_OPTEE_OS
make  clean

#####clean EDK
export GCC49_AARCH64_PREFIX=$DST_AARCH64_NONE_GCC/bin/aarch64-none-elf-

cd $DST_EDK2
export WORKSPACE=$DST_EDK2
. edksetup.sh
make -C BaseTools/Source/C clean 

######clean atf
export PATH=$DST_AARCH64_NONE_GCC/bin:$PATH
export CROSS_COMPILE=$DST_AARCH64_NONE_GCC/bin/aarch64-none-elf-
export CFLAGS='-O0 -gdwarf-2'
export DEBUG=1
export BL32=$DST_OPTEE_OS/out-os-fvp/core/tee.bin
export BL33=$DST_EDK2/Build/ArmVExpress-FVP-AArch64/RELEASE_GCC49/FV/FVP_AARCH64_EFI.fd

cd $DST_ATF
make DEBUG= FVP_TSP_RAM_LOCATION=tdram FVP_SHARED_DATA_LOCATION=tdram PLAT=fvp SPD=opteed  clean
######clean op_tee client 
export PATH=$DST_AARCH64_GCC/bin:$PATH

cd $DST_OPTEE_CLIENT
make  O=./out-client-aarch64 CROSS_COMPILE=aarch64-linux-gnu-  clean

######clean optee linux driver
export PATH=$DST_AARCH64_GCC/bin:$PATH

cd $DST_KERNEL
make V=0 ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- LOCALVERSION= M=$DST_OPTEE_LK modules clean
cd $DEV_PATH
