// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Namco system 12/23 settings device

#ifndef MAME_NAMCO_NAMCO_SETTINGS_H
#define MAME_NAMCO_NAMCO_SETTINGS_H

#pragma once

class namco_settings_device : public device_t {
public:
	namco_settings_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE_LINE_MEMBER( ce_w );
	DECLARE_WRITE_LINE_MEMBER( clk_w );
	DECLARE_WRITE_LINE_MEMBER( data_w );

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	int ce = 0, clk = 0, data = 0;
	int cur_bit = 0;
	uint8_t adr = 0, value = 0;
};

DECLARE_DEVICE_TYPE(NAMCO_SETTINGS, namco_settings_device)

#endif // MAME_NAMCO_NAMCO_SETTINGS_H
