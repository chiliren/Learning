/home/kernel/x00447438/bin/arm64_qemu/bin/qemu-system-aarch64 -m 24G -cpu cortex-a57 -M virt -nographic -smp 8 \
    -kernel $1 \
    --initrd /home/kernel/x00447438/ext/Qemu/filesystem/initrdfs/arm64/rootfs.cpio.gz \
    --append 'root=/dev/vda rdinit=/linuxrc console=ttyAMA0 sched_debug loglevel=8 earlyprintk=serial ftrace_dump_on_oops kernelcore=reliable efi_fake_mem=8G@0:0x10000 efi=debug reliable_debug=F' \
    --fsdev local,id=kmod_dev,path=../filesystem/share,security_model=none \
    -device virtio-9p-pci,fsdev=kmod_dev,mount_tag=test_mount
