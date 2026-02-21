import os
import sys
import math
import struct
import shutil
import subprocess

SECTOR_SIZE = 512
BPB_RSVD_SEC_CNT_OFFSET = 0x0E
BPB_END_OFFSET = 90


def run(cmd: list[str]) -> None:
    subprocess.run(cmd, check=True)


def dir_size_bytes(path: str) -> int:
    total = 0
    for root, _, files in os.walk(path):
        for name in files:
            full_path = os.path.join(root, name)
            if os.path.islink(full_path):
                continue
            total += os.path.getsize(full_path)
    return total


def recommended_image_size(dir_size_bytes: int) -> int:
    size = dir_size_bytes * 1.3
    size_mb = math.ceil(size / (1024 * 1024))

    # round to power of two (cleaner for FAT)
    power = 1
    while power < size_mb:
        power *= 2

    return power * 1024 * 1024


def ensure_tool(tool: str) -> None:
    if shutil.which(tool) is None:
        print(f"{sys.argv[0]}: error: required tool '{tool}' not found in PATH")
        sys.exit(1)


def copy_to_fat_image(boot_img: str, disk_dir: str) -> None:
    entries = os.listdir(disk_dir)
    if not entries:
        return

    ensure_tool("mcopy")
    for entry in entries:
        src = os.path.join(disk_dir, entry)
        run(["mcopy", "-i", boot_img, "-s", src, "::/"])


def main() -> None:
    if len(sys.argv) not in (4, 5):
        print(f"Usage: {sys.argv[0]} <boot_img> <stage0_bin> <stage1_bin> [disk_dir]")
        sys.exit(1)

    boot_img = sys.argv[1]
    stage0_bin = sys.argv[2]
    stage1_bin = sys.argv[3]
    disk_dir = sys.argv[4] if len(sys.argv) == 5 else "./disk"

    if not os.path.isdir(disk_dir):
        print(f"{sys.argv[0]}: error: disk directory not found: {disk_dir}")
        sys.exit(1)

    if os.path.isfile(boot_img):
        os.remove(boot_img)

    with open(stage0_bin, "rb") as f:
        stage0_bytes = f.read()

    if len(stage0_bytes) > SECTOR_SIZE:
        print(f"{sys.argv[0]}: error: stage0 does not fit in 512 bytes")
        sys.exit(1)

    stage0_bytes = stage0_bytes.ljust(SECTOR_SIZE, b"\x00")

    with open(stage1_bin, "rb") as f:
        stage1_bytes = f.read()

    stage1_sectors = math.ceil(len(stage1_bytes) / SECTOR_SIZE)

    if stage1_sectors > 0xFFFF:
        print(f"{sys.argv[0]}: error: size of stage1 overflows 16 bit number (size > 0xffff)")
        sys.exit(1)

    reserved_sectors = 1 + stage1_sectors

    # Create a FAT image sized to fit disk_dir with some slack.
    # Include reserved area in the sizing.
    disk_bytes = dir_size_bytes(disk_dir)
    overhead = reserved_sectors * SECTOR_SIZE + (1 * 1024 * 1024)
    image_size = recommended_image_size(disk_bytes + overhead)

    with open(boot_img, "wb") as f:
        if image_size <= 0:
            print(f"{sys.argv[0]}: error: invalid image size")
            sys.exit(1)
        f.seek(image_size - 1)
        f.write(b"\x00")

    ensure_tool("mkfs.fat")
    run(["mkfs.fat", "-R", str(reserved_sectors), "-S", str(SECTOR_SIZE), boot_img])


    with open(boot_img, "r+b") as f:
        f.seek(0)
        f.write(stage0_bytes[:3])
        f.seek(BPB_END_OFFSET)
        f.write(stage0_bytes[BPB_END_OFFSET:SECTOR_SIZE])
        f.seek(SECTOR_SIZE)
        f.write(stage1_bytes)

    copy_to_fat_image(boot_img, disk_dir)


if __name__ == "__main__":
    main()
