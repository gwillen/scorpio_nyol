UF2="build/scorpio_nyol.ino.uf2"
PUSHDIR="/media/gwillen/RPI-RP2/"
UF2CONV="/home/gwillen/.arduino15/packages/rp2040/hardware/rp2040/3.9.5/tools/uf2conv.py"
SERIAL=$(echo /dev/ttyACM*)

if [[ -d "${PUSHDIR}" ]]; then
  # the manual way (after holding 'bootsel' and whacking 'reset', and waiting for the device to mount:)
  echo "Found UF2 ready for us, copying..."
  cp "${UF2}" "${PUSHDIR}"
  echo "done!"
else
  echo "UF2 not ready, using magical script to poke the board..."
  if [[ $(echo $SERIAL | wc -w) == 1 ]]; then
    echo "Serial good: ${SERIAL} . Flashing with magic."
    python3 "${UF2CONV}" --serial "${SERIAL}" --family RP2040 --deploy build/scorpio_nyol.ino.uf2
  else
    echo "Serial bad: list of devices: ${SERIAL}. Giving up."
  fi
fi
