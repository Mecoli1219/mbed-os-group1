# ble_scan_connect.py:
from bluepy.btle import Peripheral, UUID
from bluepy.btle import Scanner, DefaultDelegate


class NotificationDelegate(DefaultDelegate):
    def __init__(self):
        DefaultDelegate.__init__(self)

    def handleNotification(self, cHandle, data):
        print("Received notification: Handle={}, Data={}".format(cHandle, data.hex()))


class ScanDelegate(DefaultDelegate):
    def __init__(self):
        DefaultDelegate.__init__(self)

    def handleDiscovery(self, dev, isNewDev, isNewData):
        if isNewDev:
            print("Discovered device", dev.addr)
        elif isNewData:
            print("Received new data from", dev.addr)


scanner = Scanner().withDelegate(ScanDelegate())
devices = scanner.scan(10.0)
n = 0
addr = []
for dev in devices:
    print("%d: Device %s (%s), RSSI=%d dB" % (n, dev.addr, dev.addrType, dev.rssi))
    addr.append(dev.addr)
    n += 1
    for adtype, desc, value in dev.getScanData():
        print(" %s = %s" % (desc, value))

number = input("Enter your device number: ")
print("Device", number)
num = int(number)
print(addr[num])
#
print("Connecting...")
dev = Peripheral(addr[num], "random")
#
print("Services...")
for svc in dev.services:
    print(str(svc))
#
try:
    testService = dev.getServiceByUUID(UUID(0xFFF0))
    for ch in testService.getCharacteristics():
        print(str(ch))

    # Set CCCD value
    ch_cccd = ch.getDescriptors(forUUID=0x2902)[0]
    ch_cccd.write(b"\x02\x00", True)  # Enable notifications

    ch = dev.getCharacteristics(uuid=UUID(0xFFF1))[0]
    if ch.supportsRead():
        print(ch.read())

    dev.setDelegate(NotificationDelegate())

    while True:
        if dev.waitForNotifications(1.0):
            continue
#
finally:
    dev.disconnect()
