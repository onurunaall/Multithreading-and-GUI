unsigned int CalculateCRC16(char* message, unsigned short length) {
	int crc = 0;
	int j = 0;
	const int poly = 0x1021;
	if (length == 0) {
		return crc;
	}
	if (message == null)
		return 0;
	while (length != 0) {
		crc ^= (message[j] << 8); //converting 8-bits of data to 16-bits of data and intermedşat crc value
		j += 1;
		for (int i = 0; i < 8; i++) {
			if ((crc & (0x8000)) != 0) {
				crc = crc << 1;
				crc ^= poly;
			}
			else {
				crc = crc << 1;
			}
		}
		length -= 1;
	}
	crc &= (0xFFFF);
}
