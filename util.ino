/*
Calculating CRCs.  Incoming commands have these appended as a way
to check quality.
http://www.excamera.com/sphinx/article-crc.html
*/
unsigned long util_crcUpdate(unsigned long crc, byte data)
{
    byte tbl_idx;
    tbl_idx = crc ^ (data >> (0 * 4));
    crc = pgm_read_dword_near(crc_table + (tbl_idx & 0x0f)) ^ (crc >> 4);
    tbl_idx = crc ^ (data >> (1 * 4));
    crc = pgm_read_dword_near(crc_table + (tbl_idx & 0x0f)) ^ (crc >> 4);
    return crc;
}

unsigned long util_crcString(String s)
{
  unsigned long crc = ~0L;
  for (int i = 0; i < s.length(); i++)
  {
    crc = util_crcUpdate(crc, s.charAt(i));
  }
  crc = ~crc;
  return crc;
}
