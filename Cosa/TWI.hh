/**
 * @file Cosa/TWI.hh
 * @version 1.0
 *
 * @section License
 * Copyright (C) 2012-2013, Mikael Patel
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 *
 * This file is part of the Arduino Che Cosa project.
 */

#ifndef __COSA_TWI_HH__
#define __COSA_TWI_HH__

#include "Cosa/Types.h"
#if !defined(__ARDUINO_TINY__)
#include "Cosa/Event.hh"

/**
 * Two wire library. Support for the I2C/TWI bus Master and Slave
 * device drivers.
 */
class TWI {
  friend void TWI_vect(void);
private:
  /**
   * Two Wire state and status codes.
   */
  enum State {
    ERROR_STATE,		// Error state
    IDLE_STATE,			// Idle state
    MT_STATE,			// Master transmit state
    MR_STATE,			// Master receive state
    ST_STATE,			// Slave transmit state
    SR_STATE			// Slave receive state
  } __attribute__((packed));

  /**
   * Macro to generate more compact status number sequence.
   */
# define TWI_STATUS(x) ((x) >> 3)

  /**
   * Two Wire AVR hardware status codes as presented to the interrupt
   * service routine.
   */
  enum {
    /** General Status Codes */
    START = TWI_STATUS(0x08),
    REP_START = TWI_STATUS(0x10),
    ARB_LOST = TWI_STATUS(0x38),
    /** Master Tranmitter Mode*/
    MT_SLA_ACK = TWI_STATUS(0x18),
    MT_SLA_NACK = TWI_STATUS(0x20),
    MT_DATA_ACK = TWI_STATUS(0x28),
    MT_DATA_NACK = TWI_STATUS(0x30),
    /** Master Receiver Mode */
    MR_SLA_ACK = TWI_STATUS(0x40),
    MR_SLA_NACK = TWI_STATUS(0x48),
    MR_DATA_ACK = TWI_STATUS(0x50),
    MR_DATA_NACK = TWI_STATUS(0x58),
    /** Slave Transmitter Mode */
    ST_SLA_ACK = TWI_STATUS(0xA8),
    ST_ARB_LOST_SLA_ACK = TWI_STATUS(0xB0),
    ST_DATA_ACK = TWI_STATUS(0xB8),
    ST_DATA_NACK = TWI_STATUS(0xC0),
    ST_LAST_DATA = TWI_STATUS(0xC8),
    /** Slave Receiver Mode */
    SR_SLA_ACK = TWI_STATUS(0x60),
    SR_ARB_LOST_SLA_ACK = TWI_STATUS(0x68),
    SR_GCALL_ACK = TWI_STATUS(0x70),
    SR_ARB_LOST_GCALL_ACK = TWI_STATUS(0x78),
    SR_DATA_ACK = TWI_STATUS(0x80),
    SR_DATA_NACK = TWI_STATUS(0x88),
    SR_GCALL_DATA_ACK = TWI_STATUS(0x90),
    SR_GCALL_DATA_NACK = TWI_STATUS(0x98),
    SR_STOP = TWI_STATUS(0xA0),
    /** Misc */
    NO_INFO = TWI_STATUS(0xF8),
    BUS_ERROR = TWI_STATUS(0x00)
  } __attribute__((packed));

  /**
   * Address mask and read/write bit.
   */
  enum {
    WRITE_OP = 0x00,		// Write operation
    READ_OP = 0x01,		// Read operation
    ADDR_MASK = 0xfe		// Address mask
  } __attribute__((packed));

  /** Default argument for start */
  static const uint8_t NEXT_IX = 255;

  /**
   * Commands for TWI hardware.
   */
  enum {
    IDLE_CMD =               _BV(TWEA) |              _BV(TWEN) | _BV(TWIE),
    START_CMD = _BV(TWINT) | _BV(TWEA) | _BV(TWSTA) | _BV(TWEN) | _BV(TWIE),
    DATA_CMD =  _BV(TWINT) |                          _BV(TWEN) | _BV(TWIE),
    ACK_CMD =   _BV(TWINT) | _BV(TWEA) |              _BV(TWEN) | _BV(TWIE),
    NACK_CMD =  _BV(TWINT) |                          _BV(TWEN) | _BV(TWIE),
    STOP_CMD =  _BV(TWINT) | _BV(TWEA) | _BV(TWSTO) | _BV(TWEN) | _BV(TWIE)
  } __attribute__((packed));

  /**
   * Device state, data buffers and target.
   */
  static const uint8_t HEADER_MAX = 4;
  static const uint8_t VEC_MAX = 4;
  uint8_t m_header[HEADER_MAX];
  iovec_t m_vec[VEC_MAX];
  Event::Handler* m_target;
  volatile State m_state;
  volatile uint8_t m_status;
  volatile uint8_t m_ix;
  volatile uint8_t* m_next;
  volatile uint8_t* m_last;
  volatile int m_count;
  uint8_t m_addr;

  /**
   * Initiate a request to the device. The address field is the bus
   * address with operation (read/write bit). Return true(1) if
   * successful otherwise false(0). 
   * @param[in] addr slave address and operation.
   * @return bool
   */
  bool request(uint8_t addr);

  /**
   * Start block transfer. Setup internal buffer pointers.
   * @param[in] state next state.
   * @param[in] ix io vector index. Default is next io vector index.
   */
  void start(State state, uint8_t ix = NEXT_IX);

  /**
   * Write next byte to hardware device and issue command. Returns
   * true if byte was written else false.
   * @param[in] cmd command to write with byte.
   * @return bool
   */
  bool write(uint8_t cmd);

  /**
   * Read next byte from hardware device and store into buffer.
   * Return true if byte was stored else false.
   * @param[in] cmd command to write (default is no command).
   * @return bool
   */
  bool read(uint8_t cmd = 0);

  /**
   * Stop block transfer and step to given state. If error sets count to
   * negative error code(-1).
   * @param[in] state to step to.
   * @param[in] type of event to push (default no event).
   */
  void stop(State state, uint8_t type = Event::NULL_TYPE);

public:
  /**
   * Device drivers are friends and may have callback/event handler
   * for completion events. 
   */
  class Driver : public Event::Handler {
    friend class TWI;
  };

  /**
   * Slave devices are friends and shall have a request handler for
   * incoming block transfers.
   */
  class Device : public Event::Handler {
    friend class TWI;
    friend void TWI_vect(void);
  protected:
    /**
     * Internal index in io-vector for read/write buffers.
     */
    static const uint8_t WRITE_IX = 0;
    static const uint8_t READ_IX = 1;

    /**
     * Filter Event::WRITE_COMPLETED_TYPE(size) and calls on_request()
     * with given write block as argument. The device is marked as ready
     * when the request has been completed and a possible result block
     * is available.
     * @param[in] type the event type.
     * @param[in] value the event value.
     */
    virtual void on_event(uint8_t type, uint16_t value);

  public:
    /**
     * Set read (result) buffer. Must be called before starting TWI.
     * @param[in] buf buffer pointer.
     * @param[in] size of buffer.
     */
    void set_read_buf(void* buf, size_t size);

    /**
     * Set write (argument) buffer. Must be called before starting TWI.
     * @param[in] buf buffer pointer.
     * @param[in] size of buffer.
     */
    void set_write_buf(void* buf, size_t size);

    /**
     * Service request callback when a write has been completed, i.e.,
     * an argument block as been written. Must be defined by sub-class.
     * Must handle write-read and write-write sequences. The device will 
     * become ready after the completion of the function. 
     * @param[in] buf buffer pointer.
     * @param[in] size of buffer.
     */
    virtual void on_request(void* buf, size_t size) = 0;
  };

  /** 
   * Construct two-wire instance. This is actually a single-ton on
   * current supported hardware, i.e. there can only be one unit.
   */
  TWI() :
    m_target(0),
    m_state(IDLE_STATE),
    m_status(NO_INFO),
    m_ix(0),
    m_next(0),
    m_last(0),
    m_count(0),
    m_addr(0)
  {
    for (uint8_t ix = 0; ix < VEC_MAX; ix++) {
      m_vec[ix].buf = 0;
      m_vec[ix].size = 0;
    }
  }

  /**
   * Start TWI bus logic. Default mode is master if address is non
   * zero. Returns true(1) if successful otherwise false(0).
   * @param[in] target receiver of events on requests.
   * @param[in] addr slave address.
   * @return true(1) if successful otherwise false(0).
   */
  bool begin(Event::Handler* target = 0, uint8_t addr = 0);
  
  /**
   * Disconnect usage of the TWI bus logic.
   * @return true(1) if successful otherwise false(0).
   */
  bool end();

  /**
   * Issue a write data request to the given slave unit. Return
   * true(1) if successful otherwise(0). 
   * @param[in] addr slave address.
   * @param[in] buf data to write.
   * @param[in] size number of bytes to write.
   * @return bool
   */
  bool write_request(uint8_t addr, void* buf, size_t size);

  /**
   * Issue a write data request to the given slave unit with given
   * byte header/command. Return true(1) if successful otherwise(0).
   * @param[in] addr slave address.
   * @param[in] header to write before buffer.
   * @param[in] buf data to write.
   * @param[in] size number of bytes to write.
   * @return bool
   */
  bool write_request(uint8_t addr, uint8_t header, void* buf, size_t size);

  /**
   * Issue a write data request to the given slave unit with given
   * header/command. Return true(1) if successful otherwise(0).
   * @param[in] addr slave address.
   * @param[in] header to write before buffer.
   * @param[in] buf data to write.
   * @param[in] size number of bytes to write.
   * @return bool
   */
  bool write_request(uint8_t addr, uint16_t header, void* buf, size_t size);

  /**
   * Issue a read data request to the given slave unit. Return true(1)
   * if successful otherwise(0). 
   * @param[in] addr slave address.
   * @param[in] buf data to write.
   * @param[in] size number of bytes to read.
   * @return number of bytes
   */
  bool read_request(uint8_t addr, void* buf, size_t size);

  /**
   * Write data to the given slave unit. Returns number of bytes
   * written or negative error code.
   * @param[in] addr slave address.
   * @param[in] buf data to write.
   * @param[in] size number of bytes to write.
   * @return number of bytes
   */
  int write(uint8_t addr, void* buf, size_t size)
  {
    if (!write_request(addr, buf, size)) return (-1);
    return (await_completed());
  }

  /**
   * Write data to the given slave unit with given byte
   * header. Returns number of bytes written or negative error code.
   * @param[in] addr slave address.
   * @param[in] header to write before buffer.
   * @param[in] buf data to write.
   * @param[in] size number of bytes to write.
   * @return number of bytes
   */
  int write(uint8_t addr, uint8_t header, void* buf = 0, size_t size = 0)
  {
    if (!write_request(addr, header, buf, size)) return (-1);
    return (await_completed());
  }

  /**
   * Write data to the given slave unit with given header. Returns
   * number of bytes written or negative error code.
   * @param[in] addr slave address.
   * @param[in] header to write before buffer.
   * @param[in] buf data to write.
   * @param[in] size number of bytes to write.
   * @return number of bytes
   */
  int write(uint8_t addr, uint16_t header, void* buf = 0, size_t size = 0)
  {
    if (!write_request(addr, header, buf, size)) return (-1);
    return (await_completed());
  }

  /**
   * Read data to the given slave unit. Returns number of bytes read
   * or negative error code.
   * @param[in] addr slave address.
   * @param[in] buf data to write.
   * @param[in] size number of bytes to read.
   * @return number of bytes
   */
  int read(uint8_t addr, void* buf, size_t size)
  {
    if (!read_request(addr, buf, size)) return (-1);
    return (await_completed());
  }

  /**
   * Await issued request to complete. Returns number of bytes 
   * or negative error code.
   */
  int await_completed(uint8_t mode = SLEEP_MODE_IDLE);
};

#else

#include "Cosa/Types.h"
#include "Cosa/Pins.hh"
#include "Cosa/Event.hh"

/**
 * Two wire library. Support for ATtiny I2C/TWI bus slave devices.
 */
class TWI {
  friend void ::USI_START_vect(void);
  friend void ::USI_OVF_vect(void);
public:
  /**
   * USI/TWI slave device. Handles incoming requests from TWI master;
   * performs address matching, byte data transfer with ack/nack,
   * and device callback. 
   */
  class Device : public Event::Handler {
    friend void ::USI_START_vect(void);
    friend void ::USI_OVF_vect(void);
  protected:
    /**
     * Filter Event::WRITE_COMPLETED_TYPE(size) and calls on_request()
     * with given write block as argument. The device is marked as
     * ready when the request has been completed and a possible
     * result block is available.
     * @param[in] type the event type.
     * @param[in] value the event value.
     */
    virtual void on_event(uint8_t type, uint16_t value);

  public:
    /**
     * Set read (result) buffer. Must be called before starting
     * TWI.
     * @param[in] buf buffer pointer.
     * @param[in] size of buffer.
     */
    void set_read_buf(void* buf, size_t size);

    /**
     * Set write (argument) buffer. Must be called before starting
     * TWI. 
     * @param[in] buf buffer pointer.
     * @param[in] size of buffer.
     */
    void set_write_buf(void* buf, size_t size);

    /**
     * Service request callback when a write has been completed,
     * i.e., an argument block as been written. Must be defined by
     * sub-class. Must handle write-read and write-write
     * sequences. The device will become ready after the completion
     * of the function.  
     * @param[in] buf buffer pointer.
     * @param[in] size of buffer.
     */
    virtual void on_request(void* buf, size_t size) = 0;
  };

  /**
   * USI TWI slave states
   */
  enum State {
    // Idle, waiting for start condition
    IDLE,
    // Check start condition
    START_CHECK,
    // Slave transmission states (Master read operation)
    READ_REQUEST,
    READ_COMPLETED,
    ACK_CHECK,
    // Slave receiver states (Master write operation)
    WRITE_REQUEST,
    WRITE_COMPLETED,
    // Slave service state (Response to write)
    SERVICE_REQUEST
  } __attribute__((packed));

private:
  /**
   * Address mask and read/write bit.
   */
  enum {
    WRITE_OP = 0x00,		// Write operation
    READ_OP = 0x01,		// Read operation
    ADDR_MASK = 0xfe		// Address mask
  } __attribute__((packed));

  /**
   * Status and Commands for USI hardware.
   */
  enum {
    // Clear all interrupt flags
    SR_CLEAR_ALL = _BV(USISIF) | _BV(USIOIF) | _BV(USIPF) | _BV(USIDC),
    // Clear all flags, except Start Condition. 
    SR_CLEAR = _BV(USIOIF) | _BV(USIPF) | _BV(USIDC),
    // Clear flags. Set USI counter to shift 1 bit (2 edges)
    SR_CLEAR_ACK = SR_CLEAR | (0x0E << USICNT0),
    // Clear flags. Set USI counter to shift 8 bits (16 edges)
    SR_CLEAR_DATA = SR_CLEAR | (0x0 << USICNT0),
    // Set USI TWI mode(0). External clock source
    CR_SERVICE_MODE = _BV(USIWM1) |  _BV(USICS1),
    // Enable start condition. Set USI TWI mode(0). External clock source
    CR_START_MODE = _BV(USISIE) | _BV(USIWM1) |  _BV(USICS1),
    // Enable start and overflow. Set USI TWI mode(1). External clock
    CR_TRANSFER_MODE = _BV(USISIE) |  _BV(USIOIE) | _BV(USIWM1) | _BV(USIWM0)
    | _BV(USICS1)
  } __attribute__((packed));

  /**
   * Device state, data buffers and target.
   */
  static const uint8_t VEC_MAX = 2;
  static const uint8_t WRITE_IX = 0;
  static const uint8_t READ_IX = 1;
  iovec_t m_vec[VEC_MAX];
  IOPin m_sda;
  IOPin m_scl;
  Device* m_target;
  volatile State m_state;
  volatile uint8_t* m_next;
  volatile uint8_t* m_last;
  volatile int m_count;
  volatile uint8_t m_addr;

  /**
   * Get current driver state.
   * @return state
   */
  State get_state()
  {
    return (m_state);
  }

  /**
   * Set driver state.
   * @param[in] state.
   */
  void set_state(State state)
  {
    m_state = state;
  }
  
  /**
   * Set data (SDA) pin input/output mode.
   * @param[in] mode.
   */
  void set_mode(IOPin::Mode mode)
  {
    m_sda.set_mode(mode);
    if (mode == IOPin::INPUT_MODE) m_sda.set();
  }

  /**
   * Set read/write buffer (WRITE(0), READ(1)).
   * @param[in] ix buffer index.
   */
  void set_buf(uint8_t ix)
  {
    if (ix > VEC_MAX) return;
    m_next = (uint8_t*) m_vec[ix].buf;
    m_last = m_next + m_vec[ix].size;
    m_count = 0;
  }

  /**
   * Return number of byte in buffer.
   * @return bytes.
   */
  uint8_t available()
  {
    return (m_last - m_next);
  }

  /**
   * Write data to buffer. Return true if not full otherwise
   * false. 
   * @param[in] data to write.
   * @return bool
   */
  bool put(uint8_t data)
  {
    *m_next++ = data;
    m_count += 1;
    return (m_next < m_last);
  }
  
  /**
   * Read data into write buffer. Return true if successful
   * otherwise if empty false. 
   * @param[out] data read.
   * @return bool
   */
  bool get(uint8_t& data)
  {
    if (m_next == m_last) return (false);
    data = *m_next++;
    m_count += 1;
    return (true);
  }

public:
  /** 
   * Construct two-wire instance. This is actually a single-ton on
   * current supported hardware, i.e. there can only be one unit.
   */
  TWI() :
    m_sda((Board::DigitalPin) Board::SDA, IOPin::INPUT_MODE, true),
    m_scl((Board::DigitalPin) Board::SCL, IOPin::OUTPUT_MODE, true),
    m_target(0),
    m_state(IDLE),
    m_next(0),
    m_last(0),
    m_count(0),
    m_addr(0)
  {
    m_sda.set();
    m_scl.set();
    for (uint8_t ix = 0; ix < VEC_MAX; ix++) {
      m_vec[ix].buf = 0;
      m_vec[ix].size = 0;
    }
  }

  /**
   * Start TWI bus logic. Returns true(1) if successful otherwise
   * false(0). 
   * @param[in] target receiver of events on requests.
   * @param[in] addr slave address.
   * @return true(1) if successful otherwise false(0).
   */
  bool begin(Device* target, uint8_t addr);
  
  /**
   * Stop TWI bus logic and disconnect. Return true(1) if successful
   * otherwise false(0).
   * @return true(1) if successful otherwise false(0).
   */
  bool end();
};
#endif

/**
 * Two-wire hardware interface module. Singleton on Arduino hardware.
*/
extern TWI twi;
#endif
