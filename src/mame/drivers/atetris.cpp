// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari, Aaron Giles
/***************************************************************************

    Atari Tetris hardware

    driver by Zsolt Vasvari

    Games supported:
        * Tetris

    Known bugs:
        * the bootlegs don't actually have the slapstic. The additional
          hardware needs to be emulated.

****************************************************************************

    Memory map

****************************************************************************

    ========================================================================
    CPU #1
    ========================================================================
    0000-0FFF   R/W   xxxxxxxx    Program RAM
    1000-1FFF   R/W   xxxxxxxx    Playfield RAM
                      xxxxxxxx       (byte 0: LSB of character code)
                      -----xxx       (byte 1: MSB of character code)
                      xxxx----       (byte 1: palette index)
    2000-20FF   R/W   xxxxxxxx    Palette RAM
                      xxx----        (red component)
                      ---xxx--       (green component)
                      ------xx       (blue component)
    2400-25FF   R/W   xxxxxxxx    EEPROM
    2800-280F   R/W   xxxxxxxx    POKEY #1
    2810-281F   R/W   xxxxxxxx    POKEY #2
    3000          W   --------    Watchdog
    3400          W   --------    EEPROM write enable
    3800          W   --------    IRQ acknowledge
    3C00          W   --xx----    Coin counters
                  W   --x-----       (right coin counter)
                  W   ---x----       (left coin counter)
    4000-7FFF   R     xxxxxxxx    Banked program ROM
    8000-FFFF   R     xxxxxxxx    Program ROM
    ========================================================================
    Interrupts:
        IRQ generated by 32V
    ========================================================================

***************************************************************************/


#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "includes/atetris.h"
#include "sound/pokey.h"
#include "machine/eeprompar.h"
#include "machine/watchdog.h"
#include "emupal.h"
#include "speaker.h"


#define MASTER_CLOCK        XTAL(14'318'181)
#define BOOTLEG_CLOCK       XTAL(14'745'600)


/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

TIMER_CALLBACK_MEMBER(atetris_state::interrupt_gen)
{
	int scanline = param;

	/* assert/deassert the interrupt */
	m_maincpu->set_input_line(0, (scanline & 32) ? ASSERT_LINE : CLEAR_LINE);

	/* set the next timer */
	scanline += 32;
	if (scanline >= 256)
		scanline -= 256;
	m_interrupt_timer->adjust(m_screen->time_until_pos(scanline), scanline);
}


WRITE8_MEMBER(atetris_state::irq_ack_w)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}



/*************************************
 *
 *  Machine init
 *
 *************************************/

void atetris_state::reset_bank()
{
	memcpy(m_slapstic_base, &m_slapstic_source[m_current_bank * 0x4000], 0x4000);
}


void atetris_state::machine_start()
{
	/* Allocate interrupt timer */
	m_interrupt_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(atetris_state::interrupt_gen),this));

	/* Set up save state */
	save_item(NAME(m_current_bank));
	machine().save().register_postload(save_prepost_delegate(FUNC(atetris_state::reset_bank), this));
}


void atetris_state::machine_reset()
{
	/* reset the slapstic */
	m_slapstic->slapstic_reset();
	m_current_bank = m_slapstic->slapstic_bank() & 1;
	reset_bank();

	/* start interrupts going (32V clocked by 16V) */
	m_interrupt_timer->adjust(m_screen->time_until_pos(48), 48);
}



/*************************************
 *
 *  Slapstic handler
 *
 *************************************/

READ8_MEMBER(atetris_state::slapstic_r)
{
	int result = m_slapstic_base[0x2000 + offset];
	int new_bank = m_slapstic->slapstic_tweak(space, offset) & 1;

	/* update for the new bank */
	if (new_bank != m_current_bank)
	{
		m_current_bank = new_bank;
		memcpy(m_slapstic_base, &m_slapstic_source[m_current_bank * 0x4000], 0x4000);
	}
	return result;
}



/*************************************
 *
 *  Coin counters
 *
 *************************************/

WRITE8_MEMBER(atetris_state::coincount_w)
{
	machine().bookkeeping().coin_counter_w(0, (data >> 5) & 1);
	machine().bookkeeping().coin_counter_w(1, (data >> 4) & 1);
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

/* full address map derived from schematics */
void atetris_state::main_map(address_map &map)
{
	map(0x0000, 0x0fff).ram();
	map(0x1000, 0x1fff).ram().w(FUNC(atetris_state::videoram_w)).share("videoram");
	map(0x2000, 0x20ff).mirror(0x0300).ram().w("palette", FUNC(palette_device::write8)).share("palette");
	map(0x2400, 0x25ff).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write));
	map(0x2800, 0x280f).mirror(0x03e0).rw("pokey1", FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x2810, 0x281f).mirror(0x03e0).rw("pokey2", FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x3000, 0x3000).mirror(0x03ff).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x3400, 0x3400).mirror(0x03ff).w("eeprom", FUNC(eeprom_parallel_28xx_device::unlock_write8));
	map(0x3800, 0x3800).mirror(0x03ff).w(FUNC(atetris_state::irq_ack_w));
	map(0x3c00, 0x3c00).mirror(0x03ff).w(FUNC(atetris_state::coincount_w));
	map(0x4000, 0x5fff).rom();
	map(0x6000, 0x7fff).r(FUNC(atetris_state::slapstic_r));
	map(0x8000, 0xffff).rom();
}


void atetris_state::atetrisb2_map(address_map &map)
{
	map(0x0000, 0x0fff).ram();
	map(0x1000, 0x1fff).ram().w(FUNC(atetris_state::videoram_w)).share("videoram");
	map(0x2000, 0x20ff).ram().w("palette", FUNC(palette_device::write8)).share("palette");
	map(0x2400, 0x25ff).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write));
	map(0x2802, 0x2802).w("sn1", FUNC(sn76496_device::command_w));
	map(0x2804, 0x2804).w("sn2", FUNC(sn76496_device::command_w));
	map(0x2806, 0x2806).w("sn3", FUNC(sn76496_device::command_w));
	map(0x2808, 0x2808).portr("IN0");
	map(0x2808, 0x280f).nopw();
	map(0x2818, 0x2818).portr("IN1");
	map(0x2818, 0x281f).nopw();
	map(0x3000, 0x3000).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x3400, 0x3400).w("eeprom", FUNC(eeprom_parallel_28xx_device::unlock_write8));
	map(0x3800, 0x3800).w(FUNC(atetris_state::irq_ack_w));
	map(0x3c00, 0x3c00).w(FUNC(atetris_state::coincount_w));
	map(0x4000, 0x5fff).rom();
	map(0x6000, 0x7fff).r(FUNC(atetris_state::slapstic_r));
	map(0x8000, 0xffff).rom();
}


void atetris_mcu_state::atetrisb3_map(address_map &map)
{
	map(0x0000, 0x0fff).ram();
	map(0x1000, 0x1fff).ram().w(FUNC(atetris_mcu_state::videoram_w)).share("videoram");
	map(0x2000, 0x20ff).ram().w("palette", FUNC(palette_device::write8)).share("palette");
	map(0x2400, 0x27ff).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write));
	map(0x2800, 0x281f).nopr().w(FUNC(atetris_mcu_state::mcu_reg_w));
	map(0x2808, 0x2808).portr("IN0");
	map(0x2818, 0x2818).portr("IN1");
	map(0x3000, 0x3000).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x3400, 0x3400).w("eeprom", FUNC(eeprom_parallel_28xx_device::unlock_write8));
	map(0x3800, 0x3800).w(FUNC(atetris_mcu_state::irq_ack_w));
	map(0x3c00, 0x3c00).w(FUNC(atetris_mcu_state::coincount_w));
	map(0x4000, 0x5fff).rom();
	map(0x6000, 0x7fff).r(FUNC(atetris_mcu_state::slapstic_r));
	map(0x8000, 0xffff).rom();
}


/*************************************
 *
 *  Bootleg MCU handlers
 *
 *************************************/

READ8_MEMBER(atetris_mcu_state::mcu_bus_r)
{
	switch (m_mcu->p2_r() & 0xf0)
	{
	case 0x40:
		return m_soundlatch[1]->read(space, 0);

	case 0xf0:
		return m_soundlatch[0]->read(space, 0);

	default:
		return 0xff;
	}
}

WRITE8_MEMBER(atetris_mcu_state::mcu_p2_w)
{
	if ((data & 0xc0) == 0x80)
		m_sn[(data >> 4) & 3]->write(m_mcu->p1_r());
}

WRITE8_MEMBER(atetris_mcu_state::mcu_reg_w)
{
	// FIXME: a lot of sound writes seem to get lost this way; why doesn't that hurt?
	m_soundlatch[0]->write(space, 0, offset | 0x20);
	m_soundlatch[1]->write(space, 0, data);
}


/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( atetris )
	// These ports are read via the Pokeys
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_DIPNAME( 0x04, 0x00, "Freeze" )            PORT_DIPLOCATION("50H:!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Freeze Step" )       PORT_DIPLOCATION("50H:!3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x00, "50H:!2" )   /* Listed As "SPARE2 (Unused)" */
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x00, "50H:!1" )   /* Listed As "SPARE1 (Unused)" */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_SERVICE( 0x80, IP_ACTIVE_HIGH )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
INPUT_PORTS_END


// Same as the regular one except they added a Flip Controls switch
static INPUT_PORTS_START( atetrisc )
	PORT_INCLUDE( atetris )

	PORT_MODIFY("IN0")
	PORT_DIPNAME( 0x20, 0x00, "Flip Controls" )     PORT_DIPLOCATION("50H:!1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics layouts
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4},
	{ 0*4*8, 1*4*8, 2*4*8, 3*4*8, 4*4*8, 5*4*8, 6*4*8, 7*4*8},
	8*8*4
};


static GFXDECODE_START( gfx_atetris )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 16 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void atetris_state::atetris_base(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, MASTER_CLOCK/8);
	m_maincpu->set_addrmap(AS_PROGRAM, &atetris_state::main_map);

	SLAPSTIC(config, m_slapstic, 101, false);

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	GFXDECODE(config, m_gfxdecode, "palette", gfx_atetris);

	PALETTE(config, "palette", 256).set_format(PALETTE_FORMAT_RRRGGGBB);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	/* note: these parameters are from published specs, not derived */
	/* the board uses an SOS-2 chip to generate video signals */
	m_screen->set_raw(MASTER_CLOCK/2, 456, 0, 336, 262, 0, 240);
	m_screen->set_screen_update(FUNC(atetris_state::screen_update));
	m_screen->set_palette("palette");

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
}

void atetris_state::atetris(machine_config &config)
{
	atetris_base(config);

	EEPROM_2804(config, "eeprom").lock_after_write(true);

	pokey_device &pokey1(POKEY(config, "pokey1", MASTER_CLOCK/8));
	pokey1.allpot_r().set_ioport("IN0");
	pokey1.add_route(ALL_OUTPUTS, "mono", 0.50);

	pokey_device &pokey2(POKEY(config, "pokey2", MASTER_CLOCK/8));
	pokey2.allpot_r().set_ioport("IN1");
	pokey2.add_route(ALL_OUTPUTS, "mono", 0.50);
}


void atetris_state::atetrisb2(machine_config &config)
{
	atetris_base(config);

	EEPROM_2804(config, "eeprom").lock_after_write(true);

	/* basic machine hardware */
	m_maincpu->set_clock(BOOTLEG_CLOCK/8);
	m_maincpu->set_addrmap(AS_PROGRAM, &atetris_state::atetrisb2_map);

	SN76489A(config, "sn1", BOOTLEG_CLOCK/8).add_route(ALL_OUTPUTS, "mono", 0.50);
	SN76489A(config, "sn2", BOOTLEG_CLOCK/8).add_route(ALL_OUTPUTS, "mono", 0.50);
	SN76489(config, "sn3", BOOTLEG_CLOCK/8).add_route(ALL_OUTPUTS, "mono", 0.50);
}


void atetris_mcu_state::atetrisb3(machine_config &config)
{
	atetris_base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &atetris_mcu_state::atetrisb3_map);

	EEPROM_2816(config, "eeprom").lock_after_write(true);

	I8749(config, m_mcu, 10_MHz_XTAL);
	m_mcu->bus_in_cb().set(FUNC(atetris_mcu_state::mcu_bus_r));
	m_mcu->bus_out_cb().set(m_soundlatch[0], FUNC(generic_latch_8_device::acknowledge_w));
	m_mcu->p2_out_cb().set(FUNC(atetris_mcu_state::mcu_p2_w));

	GENERIC_LATCH_8(config, m_soundlatch[0]);
	m_soundlatch[0]->data_pending_callback().set_inputline(m_mcu, MCS48_INPUT_IRQ);
	m_soundlatch[0]->set_separate_acknowledge(true);

	GENERIC_LATCH_8(config, m_soundlatch[1]);

	for (int i = 0; i < 4; i++)
	{
		SN76489A(config, m_sn[i], 4000000).add_route(ALL_OUTPUTS, "mono", 0.50);
	}
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( atetris )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "136066-1100.45f", 0x10000, 0x8000, CRC(2acbdb09) SHA1(5e1189227f26563fd3e5372121ea5c915620f892) )
	ROM_CONTINUE(                0x08000, 0x8000 )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "136066-1101.35a", 0x0000, 0x10000, CRC(84a1939f) SHA1(d8577985fc8ed4e74f74c68b7c00c4855b7c3270) )
ROM_END


ROM_START( atetrisa )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "d1",           0x10000, 0x8000, CRC(2bcab107) SHA1(3cfb8df8cd3782f3ff7f6b32ff15c461352061ee) )
	ROM_CONTINUE(             0x08000, 0x8000 )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "136066-1101.35a",     0x0000, 0x10000, CRC(84a1939f) SHA1(d8577985fc8ed4e74f74c68b7c00c4855b7c3270) )
ROM_END


ROM_START( atetrisb )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "tetris.01",    0x10000, 0x8000, CRC(944d15f6) SHA1(926fa5cb26b6e6a50bea455eec1f6d3fb92aa95c) )
	ROM_CONTINUE(             0x08000, 0x8000 )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "tetris.02",    0x0000, 0x10000, CRC(5c4e7258) SHA1(58060681a728e74d69b2b6f5d02faa597ca6c226) )

	/* there's an extra EEPROM, maybe used for protection crack, which */
	/* however doesn't seem to be required to run the game in this driver. */
	ROM_REGION( 0x0800, "user1", 0 )
	ROM_LOAD( "tetris.03",    0x0000, 0x0800, CRC(26618c0b) SHA1(4d6470bf3a79be3b0766e246abe00582d4c85a97) )
ROM_END


ROM_START( atetrisb2 )
	ROM_REGION( 0x18000, "maincpu", 0 ) // Some bootleg PCBs uses unmodified Atari ROMs
	ROM_LOAD( "k1-01",    0x10000, 0x8000, CRC(fa056809) SHA1(e4ccccdf9b04b68127c7b03ae263519cf00f94cb) ) // 27512
	ROM_CONTINUE(         0x08000, 0x8000 )

	ROM_REGION( 0x10000, "gfx1", 0 ) // Some bootleg PCBs uses unmodified Atari ROMs
	ROM_LOAD( "136066-1101.35a", 0x0000, 0x10000, CRC(84a1939f) SHA1(d8577985fc8ed4e74f74c68b7c00c4855b7c3270) ) // 27512

	ROM_REGION( 0x0020, "proms", 0 ) // currently unused
	ROM_LOAD( "m3-7603-5.prom1", 0x00000, 0x0020, CRC(79656af3) SHA1(bf55f100806520b291157c03999606367dd14ecc) ) // 82s123 or TBP18S030

	/* Unused. It's usual to find PLDs with different hashes, but defining equivalent equations */
	ROM_REGION( 0x859, "plds", 0 )
	ROM_LOAD( "a-gal16v8-b.bin", 0x000, 0x117, CRC(b1dfab0f) SHA1(e9e4db5459617a35a13df4b7a4586dd1b7be04ac) ) // sub PCB - Same content as "b"
	ROM_LOAD( "b-gal16v8-b.bin", 0x117, 0x117, CRC(b1dfab0f) SHA1(e9e4db5459617a35a13df4b7a4586dd1b7be04ac) ) // sub PCB - Same content as "a"
	ROM_LOAD( "c-gal16v8-b.bin", 0x22e, 0x117, CRC(e1a9db0b) SHA1(5bbac24e37a4d9b8a1387054722fa35478ca7941) ) // sub PCB
	ROM_LOAD( "1-pal16l8-a.3g" , 0x345, 0x104, CRC(dcf0d2fe) SHA1(0496acaa605ec5008b110c387136bbc714441384) ) // main PCB - Found also as GAL16v8 on some PCBs
	ROM_LOAD( "2-pal16r4-a.3r" , 0x449, 0x104, CRC(d71bdf27) SHA1(cc3503cb037de344fc353886f3492601638c9d45) ) // main PCB
	ROM_LOAD( "3-pal16r4-a.8p" , 0x54D, 0x104, CRC(e007edf2) SHA1(4f1bc31abd64e402edb4c900ddb21f258d6782c8) ) // main PCB - Found also as GAL16v8 on some PCBs
	ROM_LOAD( "4-pal16l8-a.9n" , 0x651, 0x104, CRC(3630e734) SHA1(a29dc202ffc75ac48815115b85e984fc0c9d5b59) ) // main PCB - Found also as GAL16v8 on some PCBs
	ROM_LOAD( "5-pal16l8-a.9m" , 0x755, 0x104, CRC(53b64be1) SHA1(2bf712b766541c90c38c0810ee16848e448c5205) ) // main PCB - Found also as GAL16v8 on some PCBs
ROM_END


/*
Tetris (Korean bootleg of atetrisa set)

PCB Layout
----------

RC-1108
|---------------------------------------------------|
|                                        14.31818MHz|
| PAL                                               |
|                                                   |
|     P8749H   6116                                 |
|J                                                  |
|A          10MHz                     27512         |
|M              PAL                                 |
|M                                62256             |
|A                                                  |
|                27512                              |
|                28C16                         PAL  |
|                                      PAL     PAL  |
|76489 76489  4MHz                  82S123          |
|76489              6502                            |
|VOL MB3713    PAL                                  |
|---------------------------------------------------|

A second PCB has been found with identical code, but with 1x additional SN76489AN, 1x additional DIP switch, a few more TTLs, and 6 PAL18l8ACN.
The MCU XTAL is 10.73835 MHz rather than 10 MHz on this PCB.
*/

ROM_START( atetrisb3 )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "prg.bin",           0x10000, 0x8000, CRC(2bcab107) SHA1(3cfb8df8cd3782f3ff7f6b32ff15c461352061ee) )
	ROM_CONTINUE(             0x08000, 0x8000 )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "gfx.bin",     0x0000, 0x10000, CRC(84a1939f) SHA1(d8577985fc8ed4e74f74c68b7c00c4855b7c3270) )

	// 8749 (10 MHz OSC) emulates POKEYs
	ROM_REGION( 0x0800, "mcu", 0 )
	ROM_LOAD( "8749h.bin",    0x0000, 0x0800, CRC(a66a9c47) SHA1(fbebd755a5e826c7d94ebcafdff2f9a01c9fd1a5) ) // dumped via normal methods and confirmed good via decap
	ROM_FILL( 0x06e2, 1, 0x96 ) // patch illegal opcode

	// currently unused
	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.bin", 0x00000, 0x0020, CRC(79656af3) SHA1(bf55f100806520b291157c03999606367dd14ecc) )

	ROM_REGION( 0xc00, "plds", 0 ) // all protected
	ROM_LOAD( "gal18v8a-25lp.1",   0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal18v8a-25lp.2",   0x200, 0x117, NO_DUMP )
	ROM_LOAD( "palce18v8h-25pc.3", 0x400, 0x117, NO_DUMP )
	ROM_LOAD( "palce18v8h-25pc.4", 0x600, 0x117, NO_DUMP )
	ROM_LOAD( "pal16r4b-2cn.5",    0x800, 0x104, NO_DUMP )
	ROM_LOAD( "pal16r4b-2cn.6",    0xa00, 0x104, NO_DUMP )
ROM_END

/*
atetb3482: Atari Tetris bootleg with additional UM3482 and Z80 (with its ROM)
  __________________________________________________________________
  |                                                                 |
A | ?????             74LS06   74LS197  74LS374             74LS04  |
  |                                                                 |
B |                   74LS08    74LS74  74LS374    74LS374          |
  |                                                                 |
C |                   74LS32    74LS27  74LS357    74LS374 XTAL     |
  |                                                          74LS10 |
D |                   74LS04   74LS273   74LS74    74LS374          |
  | ?? ??  74LS393   UM6116K    74LS74  74LS257      ______  74LS27 |
E |                                                  | D2  |        |
  |                   74LS245      74LS245 _______   |     | 74LS74 |
F |UM3482  74LS139    PAL16L8              |      |  |27PC |74LS161 |
  |______           __________     74LS245 | MS   |  | 512 |        |
G ||DIPS | PAL16R4  |UNPOPULAT|            | 6264 |  |_____|74LS161 |
  ||_____|          |_________|    74LS245 | L-10 |  _______        |
H |74LS04  PAL16R8  ___________            |      |  | UN  |74LS161 |
  |                 |D1 27PC512|   74LS00  |______|  | PO  |        |
I |74LS32  74LS373  |__________|   74LS32            | PU  |PAL16L8 |
  |                  __________              74LS04  | LA  |        |
J |74LS374 74LS357   | X2804AP |   74LS257   74LS138 | TED |PAL16?? |
  |____________      |_________|   74LS257   74LS257 |     |        |
K ||D3 27PC256 |                                     |_____|74LS161 |
  ||___________|     74LS245   74LS245                              |
L |________________  _______________         74LS257 74LS74 74LS161 |
  ||SHARP LH0080B  | |   UM6502A    |                               |
M ||_______________| |______________|      74LS00    74LS74 74LS161 |
  |                                                                 |
N |PAL16R4 74LS??? 4017 74LS08 74LS32 74LS04 PAL16R4 82S123 74LS32  |
  |_________________________________________________________________|
    1      2      3       4       5       6       7      8      9
*/
ROM_START( atetb3482 )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "i4-d1.bin", 0x10000, 0x8000, CRC(2acbdb09) SHA1(5e1189227f26563fd3e5372121ea5c915620f892) )
	ROM_CONTINUE(          0x08000, 0x8000 )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "f8-d2.bin", 0x0000, 0x10000, CRC(84a1939f) SHA1(d8577985fc8ed4e74f74c68b7c00c4855b7c3270) )

	ROM_REGION( 0x08000, "tunes", 0 ) // Not hooked up. Same 8K repeated four times
	ROM_LOAD( "k1-d3.bin", 0x00000, 0x08000, CRC(ce51c82b) SHA1(f90ed16f817e6b2a22b69db20348386b9c1ecb67) )

	/* Not dumped, unused */
	ROM_REGION( 0x71c, "plds", 0 )
	ROM_LOAD( "pal16r4.1n" , 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "pal16r4.7n" , 0x104, 0x104, NO_DUMP )
	ROM_LOAD( "pal16l8.9j" , 0x208, 0x104, NO_DUMP )
	ROM_LOAD( "pal16l8.9i" , 0x30c, 0x104, NO_DUMP )
	ROM_LOAD( "pal16r8.2h" , 0x410, 0x104, NO_DUMP )
	ROM_LOAD( "pal16r4.2g" , 0x514, 0x104, NO_DUMP )
	ROM_LOAD( "pal16l8.4f" , 0x618, 0x104, NO_DUMP )
ROM_END

ROM_START( atetrisc )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "tetcktl1.rom", 0x10000, 0x8000, CRC(9afd1f4a) SHA1(323d1576d92c905e8e95108b39cabf6fa0c10db6) )
	ROM_CONTINUE(             0x08000, 0x8000 )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "136066-1103.35a", 0x0000, 0x10000, CRC(ec2a7f93) SHA1(cb850141ffd1504f940fa156a39e71a4146d7fea) )
ROM_END


ROM_START( atetrisc2 )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "136066-1102.45f", 0x10000, 0x8000, CRC(1bd28902) SHA1(ae8c34f082bce1f827bf60830f207c46cb282421) )
	ROM_CONTINUE(                0x08000, 0x8000 )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "136066-1103.35a", 0x0000, 0x10000, CRC(ec2a7f93) SHA1(cb850141ffd1504f940fa156a39e71a4146d7fea) )
ROM_END



/*************************************
 *
 *  Driver init
 *
 *************************************/

void atetris_state::init_atetris()
{
	uint8_t *rgn = memregion("maincpu")->base();

	m_slapstic->slapstic_init();
	m_slapstic_source = &rgn[0x10000];
	m_slapstic_base = &rgn[0x04000];
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1988, atetris,   0,       atetris,   atetris,  atetris_state,     init_atetris, ROT0,   "Atari Games", "Tetris (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, atetrisa,  atetris, atetris,   atetris,  atetris_state,     init_atetris, ROT0,   "Atari Games", "Tetris (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, atetrisb,  atetris, atetris,   atetris,  atetris_state,     init_atetris, ROT0,   "bootleg",     "Tetris (bootleg set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, atetrisb2, atetris, atetrisb2, atetris,  atetris_state,     init_atetris, ROT0,   "bootleg",     "Tetris (bootleg set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, atetrisb3, atetris, atetrisb3, atetris,  atetris_mcu_state, init_atetris, ROT0,   "bootleg",     "Tetris (bootleg set 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, atetb3482, atetris, atetris,   atetris,  atetris_state,     init_atetris, ROT0,   "bootleg",     "Tetris (bootleg set 4, with UM3482)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1989, atetrisc,  atetris, atetris,   atetrisc, atetris_state,     init_atetris, ROT270, "Atari Games", "Tetris (cocktail set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, atetrisc2, atetris, atetris,   atetrisc, atetris_state,     init_atetris, ROT270, "Atari Games", "Tetris (cocktail set 2)", MACHINE_SUPPORTS_SAVE )
