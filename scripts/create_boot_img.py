import os
import sys
import math
import struct

SECTOR_SIZE = 512
BPB_RSVD_SEC_CNT_OFFSET = 0xe

def main():
    if len(sys.argv) != 4:
        print(f"Usage: {sys.argv[0]} <boot_img> <stage0_bin> <stage1_bin>")
        sys.exit(1)

    boot_img = sys.argv[1]
    stage0_bin = sys.argv[2]
    stage1_bin = sys.argv[3]

    if os.path.isfile(boot_img):
        os.remove(boot_img)

    with open(stage0_bin, "rb") as f:
        stage0_bytes = f.read()

    if len(stage0_bytes) > SECTOR_SIZE:
        print(f"{sys.argv[0]}: error: stage0 does not fit in 512 bytes")
        sys.exit(1)

    stage0_bytes = stage0_bytes.ljust(SECTOR_SIZE, b'\x00')

    with open(stage1_bin, "rb") as f:
        stage1_bytes = f.read()

    stage1_sectors = math.ceil(len(stage1_bytes) / SECTOR_SIZE)

    if stage1_sectors > 0xffff:
        print(f"{sys.argv[0]}: error: size of stage1 overflows 16 bit number (size > 0xffff)")
        sys.exit(1)

    with open(boot_img, "wb") as f:
        f.write(stage0_bytes)
        f.write(stage1_bytes)
        f.seek(BPB_RSVD_SEC_CNT_OFFSET)
        f.write(struct.pack("<H", stage1_sectors))


if __name__ == "__main__":
    main()
