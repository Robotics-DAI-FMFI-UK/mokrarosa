#define SERIAL_STATE_IDLE      0
#define SERIAL_STATE_RECEIVING 1
#define SERIAL_BUFFER_LENGTH   20

static volatile uint8_t serial_state;
static uint8_t serial_buffer[SERIAL_BUFFER_LENGTH];
static volatile uint8_t serial_buf_wp, serial_buf_rp;

static volatile uint8_t receiving_byte;

static volatile uint32_t time_startbit_noticed;
static volatile uint8_t next_bit_order;
static volatile uint8_t waiting_stop_bit;
static uint16_t one_byte_duration;
static uint16_t one_bit_duration;
static uint16_t one_bit_write_duration;
static uint16_t half_of_one_bit_duration;

void setup()
{
  init_serial(9600);
}

void loop()
{
  if (serial_available())
    serial_write(serial_read());
}


void init_serial(uint32_t baud_rate)
{
  pinMode(2, INPUT);
  pinMode(4, OUTPUT);

  serial_state = SERIAL_STATE_IDLE;

  one_byte_duration = 9500000 / baud_rate;
  one_bit_duration = 1000000 / baud_rate;
  one_bit_write_duration = one_bit_duration - 1;
  half_of_one_bit_duration = 500000 / baud_rate;

  PCMSK2 |= 4; //PCINT18;
  PCIFR &= ~4; //PCIF2;
  PCICR |= 4; // PCIE2;
}

ISR(PCINT2_vect)
{
  uint32_t tm = micros();
  if (serial_state == SERIAL_STATE_IDLE)
  {
    time_startbit_noticed = tm;
    serial_state = SERIAL_STATE_RECEIVING;
    receiving_byte = 0xFF;
    next_bit_order = 0;
  }
  else if (tm - time_startbit_noticed > one_byte_duration)
  {
    serial_buffer[serial_buf_wp] = receiving_byte;
    serial_buf_wp++;
    if (serial_buf_wp == SERIAL_BUFFER_LENGTH) serial_buf_wp = 0;
    time_startbit_noticed = tm;
    receiving_byte = 0xFF;
    next_bit_order = 0;
  }
  else if (PIND & 4)
  {
    int8_t new_next_bit_order = (tm - time_startbit_noticed - half_of_one_bit_duration) / one_bit_duration;
    while (next_bit_order < new_next_bit_order)
    {
      receiving_byte &= ~(1 << next_bit_order);
      next_bit_order++;
    }
    if (next_bit_order == 8)
    {
      serial_buffer[serial_buf_wp] = receiving_byte;
      serial_buf_wp++;
      if (serial_buf_wp == SERIAL_BUFFER_LENGTH) serial_buf_wp = 0;
      serial_state = SERIAL_STATE_IDLE;
    }
  } else
    next_bit_order = (tm - time_startbit_noticed - half_of_one_bit_duration) / one_bit_duration;
}

uint8_t serial_available()
{
  cli();
  if (serial_buf_rp != serial_buf_wp)
  {
    sei();
    return 1;
  }
  if (serial_state == SERIAL_STATE_RECEIVING)
  {
    uint32_t tm = micros();
    if (tm - time_startbit_noticed > one_byte_duration)
    {
      serial_state = SERIAL_STATE_IDLE;
      serial_buffer[serial_buf_wp] = receiving_byte;
      serial_buf_wp++;
      if (serial_buf_wp == SERIAL_BUFFER_LENGTH) serial_buf_wp = 0;
      sei();
      return 1;
    }
  }
  sei();
  return 0;
}

int16_t serial_read()
{
  cli();
  if (serial_buf_rp != serial_buf_wp)
  {
    uint8_t ch = serial_buffer[serial_buf_rp];
    serial_buf_rp++;
    if (serial_buf_rp == SERIAL_BUFFER_LENGTH) serial_buf_rp = 0;
    sei();
    return ch;
  }

  if (serial_state == SERIAL_STATE_RECEIVING)
  {
    uint32_t tm = micros();
    if (tm - time_startbit_noticed > one_byte_duration)
    {
      uint8_t ch = receiving_byte;
      serial_state = SERIAL_STATE_IDLE;
      sei();
      return ch;
    }
  }
  sei();
  return -1;
}

void serial_write(uint8_t ch)
{
#ifdef ECHO_BT_TO_USB
  Serial.print((char)ch);
#endif
  PORTD &= ~16;
  delayMicroseconds(one_bit_write_duration);
  for (uint8_t i = 0; i < 8; i++)
  {
    if (ch & 1) PORTD |= 16;
    else PORTD &= ~16;
    ch >>= 1;
    delayMicroseconds(one_bit_write_duration);
  }
  PORTD |= 16;
  delayMicroseconds(one_bit_write_duration);
  delayMicroseconds(one_bit_write_duration);
  delayMicroseconds(one_bit_write_duration);
  delayMicroseconds(one_bit_write_duration);
  delayMicroseconds(one_bit_write_duration);
}

uint16_t serial_readln(uint8_t *ln, uint16_t max_length)
{
  uint16_t len;
  int16_t ch;
  do {
    ch = serial_read();
    if (ch == 13) continue;
  } while (ch == -1);

  do {
    if ((ch != 13) && (ch != 10) && (ch != -1))
    {
      *(ln++) = ch;
      max_length--;
      len++;
    }
    ch = serial_read();
  } while ((ch != 13) && max_length);
  *ln = 0;
  return len;
}

void serial_print_num(int32_t number)
{
  if (number < 0)
  {
    serial_write('-');
    number = -number;
  }
  int32_t rad = 1;
  while (number / rad) rad *= 10;
  if (number > 0) rad /= 10;
  while (rad)
  {
    serial_write((char)('0' + (number / rad)));
    number -= (number / rad) * rad;
    rad /= 10;
  }
}

void serial_print_char(char ch)
{
  serial_write(ch);
}

void serial_print(const char *str)
{
  while (*str) serial_write(*(str++));
}

void serial_println(const char *str)
{
  serial_print(str);
  serial_write(13);
  serial_write(10);
}

void serial_print_flash(const char *str)
{
  int ln = strlen_P(str);
  for (int i = 0; i < ln; i++)
    serial_write(pgm_read_byte(str + i));
}

void serial_println_flash(const char *str)
{
  serial_print_flash(str);
  serial_write(13);
  serial_write(10);
}

void serial_println_num(int32_t number)
{
  serial_print_num(number);
  serial_println();
}

void serial_println_char(char ch)
{
  serial_write(ch);
  serial_println();
}

void serial_println()
{
  serial_write(13);
  serial_write(10);
}
