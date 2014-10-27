#!/bin/zsh
# Use as:
#
#  makejpgs.zsh <blockdevice> <offset> <size-runway>
#
#  <blockdeivce> is a block device which was used as the
#  parameter to jaypack
#  <offset> is the offset passed to jaypack
#  <size-runway> is how much extra bytes to transfer past 
#  the end. i recommend leaving some extra space, like 32
#  bytes, just to be sure. there was a bug in jaypack earlier
#  which would output a size that was 2 shorter than it should
i=0
while read line; do
  thing=("${(s/ /)line}")
  off=$thing[2]
  count=$thing[3]
  dd if=$1 bs=1 skip=$(($off + $2)) count=$(($count + $3)) of=jpgs/$off.jpg 1>/dev/null 2>&1
  ((i++))
  echo Processed image $i, offset $off, size $(($count / 1024))K, currently at $(($off / 1048576))M
done
