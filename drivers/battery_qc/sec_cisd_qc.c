/*
 *  sec_cisd_qc.c
 *  Samsung Mobile Battery Driver
 *
 *  Copyright (C) 2012 Samsung Electronics
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include "include/sec_battery_qc.h"
#include "include/sec_cisd_qc.h"

#if defined(CONFIG_SEC_ABC)
#include <linux/sti/abc_common.h>
#endif
#if defined(CONFIG_QPNP_SMB5)
#if defined(CONFIG_SEC_A90Q_PROJECT) || defined(CONFIG_SEC_A70S_PROJECT)
#include "../power/supply/qcom_r1/smb5-lib.h"
#else
#include "../power/supply/qcom/smb5-lib.h"
#endif
#endif

const char *cisd_data_str[] = {
	"RESET_ALG", "ALG_INDEX", "FULL_CNT", "CAP_MAX", "CAP_MIN", "RECHARGING_CNT", "VALERT_CNT",
	"BATT_CYCLE", "WIRE_CNT", "WIRELESS_CNT", "HIGH_SWELLING_CNT", "LOW_SWELLING_CNT",
	"SWELLING_CHARGING", "SWELLING_FULL_CNT", "SWELLING_RECOVERY_CNT", "AICL_CNT", "BATT_THM_MAX",
	"BATT_THM_MIN", "CHG_THM_MAX", "CHG_THM_MIN", "WPC_THM_MAX", "WPC_THM_MIN", "USB_THM_MAX", "USB_THM_MIN",
	"CHG_BATT_THM_MAX", "CHG_BATT_THM_MIN", "CHG_CHG_THM_MAX", "CHG_CHG_THM_MIN", "CHG_WPC_THM_MAX",
	"CHG_WPC_THM_MIN", "CHG_USB_THM_MAX", "CHG_USB_THM_MIN", "USB_OVERHEAT_CHARGING", "UNSAFETY_VOLT",
	"UNSAFETY_TEMP", "SAFETY_TIMER", "VSYS_OVP", "VBAT_OVP", "USB_OVERHEAT_RAPID_CHANGE", "BUCK_OFF",
	"USB_OVERHEAT_ALONE", "DROP_SENSOR"
};
const char *cisd_data_str_d[] = {
	"FULL_CNT_D", "CAP_MAX_D", "CAP_MIN_D", "RECHARGING_CNT_D", "VALERT_CNT_D", "WIRE_CNT_D", "WIRELESS_CNT_D",
	"HIGH_SWELLING_CNT_D", "LOW_SWELLING_CNT_D", "SWELLING_CHARGING_D", "SWELLING_FULL_CNT_D",
	"SWELLING_RECOVERY_CNT_D", "AICL_CNT_D", "BATT_THM_MAX_D", "BATT_THM_MIN_D", "CHG_THM_MAX_D",
	"CHG_THM_MIN_D", "WPC_THM_MAX_D", "WPC_THM_MIN_D", "USB_THM_MAX_D", "USB_THM_MIN_D",
	"CHG_BATT_THM_MAX_D", "CHG_BATT_THM_MIN_D", "CHG_CHG_THM_MAX_D", "CHG_CHG_THM_MIN_D",
	"CHG_WPC_THM_MAX_D", "CHG_WPC_THM_MIN_D", "CHG_USB_THM_MAX_D", "CHG_USB_THM_MIN_D",
	"USB_OVERHEAT_CHARGING_D", "UNSAFETY_VOLT_D", "UNSAFETY_TEMP_D", "SAFETY_TIMER_D", "VSYS_OVP_D",
	"VBAT_OVP_D", "USB_OVERHEAT_RAPID_CHANGE_D", "BUCK_OFF_D", "USB_OVERHEAT_ALONE_D", "DROP_SENSOR_D"
};

const char *cisd_event_data_str[] = {"DC_ERR", "TA_OCP_DET", "TA_OCP_ON"};

bool sec_bat_cisd_check(struct sec_battery_info *battery)
{
//	union power_supply_propval capcurr_val = {0, };
//	union power_supply_propval vbat_val = {0, };
	struct cisd *pcisd = &battery->cisd;
	bool ret = false;
	struct smb_charger *chg;

	if (battery->factory_mode || battery->is_jig_on || battery->skip_cisd) {
		dev_info(battery->dev, "%s: No need to check in factory mode\n",
			__func__);
		return ret;
	}

	chg = power_supply_get_drvdata(battery->psy_bat);
	
	if ((battery->prev_status == POWER_SUPPLY_STATUS_CHARGING) && (battery->status == POWER_SUPPLY_STATUS_FULL)) {
		if(battery->current_event & SEC_BAT_CURRENT_EVENT_HIGH_TEMP_SWELLING) {
			pcisd->data[CISD_DATA_SWELLING_FULL_CNT]++;
			pcisd->data[CISD_DATA_SWELLING_FULL_CNT_PER_DAY]++;
		} else {
			pcisd->data[CISD_DATA_FULL_COUNT]++;
			pcisd->data[CISD_DATA_FULL_COUNT_PER_DAY]++;
		}
	//	pr_info("%s: CISD_DATA_FULL_COUNT :%d\n", __func__, battery->cisd.data[CISD_DATA_FULL_COUNT]);
	}


	if ((battery->prev_status == POWER_SUPPLY_STATUS_FULL) && (battery->status == POWER_SUPPLY_STATUS_FULL) 
		&& (battery->prev_charge_type == POWER_SUPPLY_CHARGE_TYPE_NONE) && (battery->charge_type > POWER_SUPPLY_CHARGE_TYPE_NONE))
	{
		battery->cisd.data[CISD_DATA_RECHARGING_COUNT]++;
		battery->cisd.data[CISD_DATA_RECHARGING_COUNT_PER_DAY]++;
	//	pr_info("%s: CISD_DATA_RECHARGING_COUNT :%d, \n", __func__, battery->cisd.data[CISD_DATA_RECHARGING_COUNT]);
	}

	if ((battery->prev_current_event & SEC_BAT_CURRENT_EVENT_VBAT_OVP)
		!= (battery->current_event & SEC_BAT_CURRENT_EVENT_VBAT_OVP)) {
		if(battery->current_event & SEC_BAT_CURRENT_EVENT_VBAT_OVP) {
			pcisd->data[CISD_DATA_VBAT_OVP]++;
			pcisd->data[CISD_DATA_VBAT_OVP_PER_DAY]++;
		}
	}

	if ((battery->prev_current_event & SEC_BAT_CURRENT_EVENT_SWELLING_MODE)
		!= (battery->current_event & SEC_BAT_CURRENT_EVENT_SWELLING_MODE)) {	
		if (battery->current_event & SEC_BAT_CURRENT_EVENT_HIGH_TEMP_SWELLING) {
			pcisd->data[CISD_DATA_HIGH_TEMP_SWELLING]++;
			pcisd->data[CISD_DATA_HIGH_TEMP_SWELLING_PER_DAY]++;
		//	pr_info("%s: CISD_DATA_HIGH_TEMP_SWELLING :%d\n", __func__, pcisd->data[CISD_DATA_HIGH_TEMP_SWELLING]);
		} else if (battery->current_event & SEC_BAT_CURRENT_EVENT_LOW_TEMP_SWELLING) {
			pcisd->data[CISD_DATA_LOW_TEMP_SWELLING]++;
			pcisd->data[CISD_DATA_LOW_TEMP_SWELLING_PER_DAY]++;
		//	pr_info("%s: CISD_DATA_LOW_TEMP_SWELLING :%d\n", __func__, pcisd->data[CISD_DATA_LOW_TEMP_SWELLING]);
		} else if ((battery->prev_current_event & SEC_BAT_CURRENT_EVENT_HIGH_TEMP_SWELLING)
			&& (!(battery->current_event & SEC_BAT_CURRENT_EVENT_HIGH_TEMP_SWELLING))) {
			pcisd->data[CISD_DATA_SWELLING_RECOVERY_CNT]++;
			pcisd->data[CISD_DATA_SWELLING_RECOVERY_CNT_PER_DAY]++;
		//	pr_info("%s: CISD_DATA_SWELLING_RECOVERY_CNT :%d\n", __func__, pcisd->data[CISD_DATA_SWELLING_RECOVERY_CNT]);
		} 
	}

	if (battery->prev_batt_health != battery->batt_health) {
		if ((battery->batt_health == POWER_SUPPLY_HEALTH_OVERHEAT) ||
			(battery->batt_health == POWER_SUPPLY_HEALTH_COLD) ||
			(battery->batt_health == POWER_SUPPLY_HEALTH_OVERHEATLIMIT)) {
				pr_info("%s: Unsafe Temperature(%d)\n", __func__, battery->batt_health);
				pcisd->data[CISD_DATA_UNSAFETY_TEMPERATURE]++;
				pcisd->data[CISD_DATA_UNSAFE_TEMPERATURE_PER_DAY]++;
		}
	}
	
//	pr_info("%s:  skip_cisd: %d, battery->voltage_now:%d, max_voltage_thr:%d\n", __func__, battery->skip_cisd, battery->v_now, pcisd->max_voltage_thr);

	
	if ((battery->status == POWER_SUPPLY_STATUS_CHARGING) ||
		(battery->status == POWER_SUPPLY_STATUS_FULL)) {
		/* check abnormal vbat */
		pcisd->ab_vbat_check_count = battery->v_now > pcisd->max_voltage_thr ?
				pcisd->ab_vbat_check_count + 1 : 0;

		if ((pcisd->ab_vbat_check_count >= pcisd->ab_vbat_max_count) &&
			!(pcisd->state & CISD_STATE_OVER_VOLTAGE)) {
			dev_info(battery->dev, "%s : [CISD] Battery Over Voltage Protction !! vbat(%d)mV\n",
				__func__, battery->v_now);
#if 0 //need to check cisd
			vbat_val.intval = true;
			psy_do_property("battery", set, POWER_SUPPLY_EXT_PROP_VBAT_OVP,
					vbat_val);
#endif
			pcisd->data[CISD_DATA_VBAT_OVP]++;
			pcisd->data[CISD_DATA_VBAT_OVP_PER_DAY]++;
			pcisd->state |= CISD_STATE_OVER_VOLTAGE;
#if defined(CONFIG_SEC_ABC)
			sec_abc_send_event("MODULE=battery@ERROR=over_voltage");
#endif
		}

		if (battery->batt_temp > pcisd->data[CISD_DATA_CHG_BATT_TEMP_MAX])
			pcisd->data[CISD_DATA_CHG_BATT_TEMP_MAX] = battery->batt_temp;
		if (battery->batt_temp < pcisd->data[CISD_DATA_CHG_BATT_TEMP_MIN])
			pcisd->data[CISD_DATA_CHG_BATT_TEMP_MIN] = battery->batt_temp;

		if (battery->die_temp > pcisd->data[CISD_DATA_CHG_CHG_TEMP_MAX])
			pcisd->data[CISD_DATA_CHG_CHG_TEMP_MAX] = battery->die_temp;
		if (battery->die_temp < pcisd->data[CISD_DATA_CHG_CHG_TEMP_MIN])
			pcisd->data[CISD_DATA_CHG_CHG_TEMP_MIN] = battery->die_temp;

#if 0  /*don't use*/
		if (battery->wpc_temp > pcisd->data[CISD_DATA_CHG_WPC_TEMP_MAX])
			pcisd->data[CISD_DATA_CHG_WPC_TEMP_MAX] = battery->wpc_temp;
		if (battery->wpc_temp < pcisd->data[CISD_DATA_CHG_WPC_TEMP_MIN])
			pcisd->data[CISD_DATA_CHG_WPC_TEMP_MIN] = battery->wpc_temp;

		if (battery->usb_temp > pcisd->data[CISD_DATA_CHG_USB_TEMP_MAX])
			pcisd->data[CISD_DATA_CHG_USB_TEMP_MAX] = battery->usb_temp;
		if (battery->usb_temp < pcisd->data[CISD_DATA_CHG_USB_TEMP_MIN])
			pcisd->data[CISD_DATA_CHG_USB_TEMP_MIN] = battery->usb_temp;
#endif

		if (battery->batt_temp > pcisd->data[CISD_DATA_CHG_BATT_TEMP_MAX_PER_DAY])
			pcisd->data[CISD_DATA_CHG_BATT_TEMP_MAX_PER_DAY] = battery->batt_temp;
		if (battery->batt_temp < pcisd->data[CISD_DATA_CHG_BATT_TEMP_MIN_PER_DAY])
			pcisd->data[CISD_DATA_CHG_BATT_TEMP_MIN_PER_DAY] = battery->batt_temp;

		if (battery->die_temp > pcisd->data[CISD_DATA_CHG_CHG_TEMP_MAX_PER_DAY])
			pcisd->data[CISD_DATA_CHG_CHG_TEMP_MAX_PER_DAY] = battery->die_temp;
		if (battery->die_temp < pcisd->data[CISD_DATA_CHG_CHG_TEMP_MIN_PER_DAY])
			pcisd->data[CISD_DATA_CHG_CHG_TEMP_MIN_PER_DAY] = battery->die_temp;

#if 0  /*don't use*/
		if (battery->wpc_temp > pcisd->data[CISD_DATA_CHG_WPC_TEMP_MAX_PER_DAY])
			pcisd->data[CISD_DATA_CHG_WPC_TEMP_MAX_PER_DAY] = battery->wpc_temp;
		if (battery->wpc_temp < pcisd->data[CISD_DATA_CHG_WPC_TEMP_MIN_PER_DAY])
			pcisd->data[CISD_DATA_CHG_WPC_TEMP_MIN_PER_DAY] = battery->wpc_temp;

		if (battery->usb_temp > pcisd->data[CISD_DATA_CHG_USB_TEMP_MAX_PER_DAY])
			pcisd->data[CISD_DATA_CHG_USB_TEMP_MAX_PER_DAY] = battery->usb_temp;
		if (battery->usb_temp < pcisd->data[CISD_DATA_CHG_USB_TEMP_MIN_PER_DAY])
			pcisd->data[CISD_DATA_CHG_USB_TEMP_MIN_PER_DAY] = battery->usb_temp;

		if (battery->usb_temp > 800 && !battery->usb_overheat_check) {
			battery->cisd.data[CISD_DATA_USB_OVERHEAT_CHARGING]++;
			battery->cisd.data[CISD_DATA_USB_OVERHEAT_CHARGING_PER_DAY]++;
			battery->usb_overheat_check = true;
		}
#endif
	} else  {
		/* discharging */
		if (battery->status == POWER_SUPPLY_STATUS_NOT_CHARGING) {
			/* check abnormal vbat */
			pcisd->ab_vbat_check_count = battery->v_now > pcisd->max_voltage_thr ?
				pcisd->ab_vbat_check_count + 1 : 0;

			if ((pcisd->ab_vbat_check_count >= pcisd->ab_vbat_max_count) &&
				!(pcisd->state & CISD_STATE_OVER_VOLTAGE)) {
				pcisd->data[CISD_DATA_VBAT_OVP]++;
				pcisd->data[CISD_DATA_VBAT_OVP_PER_DAY]++;
				pcisd->state |= CISD_STATE_OVER_VOLTAGE;
#if defined(CONFIG_SEC_ABC)
				sec_abc_send_event("MODULE=battery@ERROR=over_voltage");
#endif
			}
		}
#if 0 // need to check cisd
		capcurr_val.intval = SEC_BATTERY_CAPACITY_FULL;
		psy_do_property(battery->pdata->fuelgauge_name, get,
			POWER_SUPPLY_PROP_ENERGY_NOW, capcurr_val);

		if (capcurr_val.intval == -1) {
			dev_info(battery->dev, "%s: [CISD] FG I2C fail. skip cisd check \n", __func__);
			return ret;
		}

		if (capcurr_val.intval > pcisd->data[CISD_DATA_CAP_MAX])
			pcisd->data[CISD_DATA_CAP_MAX] = capcurr_val.intval;
		if (capcurr_val.intval < pcisd->data[CISD_DATA_CAP_MIN])
			pcisd->data[CISD_DATA_CAP_MIN] = capcurr_val.intval;

		if (capcurr_val.intval > pcisd->data[CISD_DATA_CAP_MAX_PER_DAY])
			pcisd->data[CISD_DATA_CAP_MAX_PER_DAY] = capcurr_val.intval;
		if (capcurr_val.intval < pcisd->data[CISD_DATA_CAP_MIN_PER_DAY])
			pcisd->data[CISD_DATA_CAP_MIN_PER_DAY] = capcurr_val.intval;
#endif
	}

	if (battery->batt_temp > pcisd->data[CISD_DATA_BATT_TEMP_MAX])
		pcisd->data[CISD_DATA_BATT_TEMP_MAX] = battery->batt_temp;
	if (battery->batt_temp < battery->cisd.data[CISD_DATA_BATT_TEMP_MIN])
		pcisd->data[CISD_DATA_BATT_TEMP_MIN] = battery->batt_temp;

	if (battery->die_temp > pcisd->data[CISD_DATA_CHG_TEMP_MAX])
		pcisd->data[CISD_DATA_CHG_TEMP_MAX] = battery->die_temp;
	if (battery->die_temp < pcisd->data[CISD_DATA_CHG_TEMP_MIN])
		pcisd->data[CISD_DATA_CHG_TEMP_MIN] = battery->die_temp;

#if 0  /*don't use*/
	if (battery->wpc_temp > pcisd->data[CISD_DATA_WPC_TEMP_MAX])
		pcisd->data[CISD_DATA_WPC_TEMP_MAX] = battery->wpc_temp;
	if (battery->wpc_temp < battery->cisd.data[CISD_DATA_WPC_TEMP_MIN])
		pcisd->data[CISD_DATA_WPC_TEMP_MIN] = battery->wpc_temp;

	if (battery->usb_temp > pcisd->data[CISD_DATA_USB_TEMP_MAX])
		pcisd->data[CISD_DATA_USB_TEMP_MAX] = battery->usb_temp;
	if (battery->usb_temp < pcisd->data[CISD_DATA_USB_TEMP_MIN])
		pcisd->data[CISD_DATA_USB_TEMP_MIN] = battery->usb_temp;
#endif

	if (battery->batt_temp > pcisd->data[CISD_DATA_BATT_TEMP_MAX_PER_DAY])
		pcisd->data[CISD_DATA_BATT_TEMP_MAX_PER_DAY] = battery->batt_temp;
	if (battery->batt_temp < pcisd->data[CISD_DATA_BATT_TEMP_MIN_PER_DAY])
		pcisd->data[CISD_DATA_BATT_TEMP_MIN_PER_DAY] = battery->batt_temp;

	if (battery->die_temp > pcisd->data[CISD_DATA_CHG_TEMP_MAX_PER_DAY])
		pcisd->data[CISD_DATA_CHG_TEMP_MAX_PER_DAY] = battery->die_temp;
	if (battery->die_temp < pcisd->data[CISD_DATA_CHG_TEMP_MIN_PER_DAY])
		pcisd->data[CISD_DATA_CHG_TEMP_MIN_PER_DAY] = battery->die_temp;

#if 0  /*don't use*/
	if (battery->wpc_temp > pcisd->data[CISD_DATA_WPC_TEMP_MAX_PER_DAY])
		pcisd->data[CISD_DATA_WPC_TEMP_MAX_PER_DAY] = battery->wpc_temp;
	if (battery->wpc_temp < pcisd->data[CISD_DATA_WPC_TEMP_MIN_PER_DAY])
		pcisd->data[CISD_DATA_WPC_TEMP_MIN_PER_DAY] = battery->wpc_temp;

	if (battery->usb_temp > pcisd->data[CISD_DATA_USB_TEMP_MAX_PER_DAY])
		pcisd->data[CISD_DATA_USB_TEMP_MAX_PER_DAY] = battery->usb_temp;
	if (battery->usb_temp < pcisd->data[CISD_DATA_USB_TEMP_MIN_PER_DAY])
		pcisd->data[CISD_DATA_USB_TEMP_MIN_PER_DAY] = battery->usb_temp;
#endif

	return ret;
}

struct cisd *gcisd;
void sec_battery_cisd_init(struct sec_battery_info *battery)
{
	battery->cisd.state = CISD_STATE_NONE;

	battery->cisd.data[CISD_DATA_ALG_INDEX] = battery->cisd_alg_index;
	battery->cisd.data[CISD_DATA_FULL_COUNT] = 1;
	battery->cisd.data[CISD_DATA_BATT_TEMP_MAX] = -300;
	battery->cisd.data[CISD_DATA_CHG_TEMP_MAX] = -300;
	battery->cisd.data[CISD_DATA_WPC_TEMP_MAX] = -300;
	battery->cisd.data[CISD_DATA_USB_TEMP_MAX] = -300;
	battery->cisd.data[CISD_DATA_BATT_TEMP_MIN] = 1000;
	battery->cisd.data[CISD_DATA_CHG_TEMP_MIN] = 1000;
	battery->cisd.data[CISD_DATA_WPC_TEMP_MIN] = 1000;
	battery->cisd.data[CISD_DATA_USB_TEMP_MIN] = 1000;
	battery->cisd.data[CISD_DATA_CHG_BATT_TEMP_MAX] = -300;
	battery->cisd.data[CISD_DATA_CHG_CHG_TEMP_MAX] = -300;
	battery->cisd.data[CISD_DATA_CHG_WPC_TEMP_MAX] = -300;
	battery->cisd.data[CISD_DATA_CHG_USB_TEMP_MAX] = -300;
	battery->cisd.data[CISD_DATA_CHG_BATT_TEMP_MIN] = 1000;
	battery->cisd.data[CISD_DATA_CHG_CHG_TEMP_MIN] = 1000;
	battery->cisd.data[CISD_DATA_CHG_WPC_TEMP_MIN] = 1000;
	battery->cisd.data[CISD_DATA_CHG_USB_TEMP_MIN] = 1000;	
	battery->cisd.data[CISD_DATA_CAP_MIN] = 0xFFFF;

	battery->cisd.data[CISD_DATA_FULL_COUNT_PER_DAY] = 1;
	battery->cisd.data[CISD_DATA_BATT_TEMP_MAX_PER_DAY] = -300;
	battery->cisd.data[CISD_DATA_CHG_TEMP_MAX_PER_DAY] = -300;
	battery->cisd.data[CISD_DATA_WPC_TEMP_MAX_PER_DAY] = -300;
	battery->cisd.data[CISD_DATA_USB_TEMP_MAX_PER_DAY] = -300;
	battery->cisd.data[CISD_DATA_BATT_TEMP_MIN_PER_DAY] = 1000;
	battery->cisd.data[CISD_DATA_CHG_TEMP_MIN_PER_DAY] = 1000;
	battery->cisd.data[CISD_DATA_WPC_TEMP_MIN_PER_DAY] = 1000;
	battery->cisd.data[CISD_DATA_USB_TEMP_MIN_PER_DAY] = 1000;

	battery->cisd.data[CISD_DATA_CHG_BATT_TEMP_MAX_PER_DAY] = -300;
	battery->cisd.data[CISD_DATA_CHG_CHG_TEMP_MAX_PER_DAY] = -300;
	battery->cisd.data[CISD_DATA_CHG_WPC_TEMP_MAX_PER_DAY] = -300;
	battery->cisd.data[CISD_DATA_CHG_USB_TEMP_MAX_PER_DAY] = -300;
	battery->cisd.data[CISD_DATA_CHG_BATT_TEMP_MIN_PER_DAY] = 1000;
	battery->cisd.data[CISD_DATA_CHG_CHG_TEMP_MIN_PER_DAY] = 1000;
	battery->cisd.data[CISD_DATA_CHG_WPC_TEMP_MIN_PER_DAY] = 1000;
	battery->cisd.data[CISD_DATA_CHG_USB_TEMP_MIN_PER_DAY] = 1000;

	battery->cisd.ab_vbat_max_count = 2; /* should be 2 */
	battery->cisd.ab_vbat_check_count = 0;
	battery->cisd.max_voltage_thr = battery->max_voltage_thr;

	/* set cisd pointer */
	gcisd = &battery->cisd;

	/* initialize pad data */
	mutex_init(&battery->cisd.padlock);
	init_cisd_pad_data(&battery->cisd);
}

static struct pad_data* create_pad_data(unsigned int pad_id, unsigned int pad_count)
{
	struct pad_data* temp_data;

	temp_data = kzalloc(sizeof(struct pad_data), GFP_KERNEL);
	if (temp_data == NULL)
		return NULL;

	temp_data->id = pad_id;
	temp_data->count = pad_count;
	temp_data->prev = temp_data->next = NULL;

	return temp_data;
}

static struct pad_data* find_pad_data_by_id(struct cisd* cisd, unsigned int pad_id)
{
	struct pad_data* temp_data = cisd->pad_array->next;

	if (cisd->pad_count <= 0 || temp_data == NULL)
		return NULL;

	while ((temp_data->id != pad_id) &&
		((temp_data = temp_data->next) != NULL));

	return temp_data;
}

static void add_pad_data(struct cisd* cisd, unsigned int pad_id, unsigned int pad_count)
{
	struct pad_data* temp_data = cisd->pad_array->next;
	struct pad_data* pad_data;

	if (pad_id == 0 || pad_id >= MAX_PAD_ID)
		return;

	pad_data = create_pad_data(pad_id, pad_count);
	if (pad_data == NULL)
		return;

	pr_info("%s: id(0x%x), count(%d)\n", __func__, pad_id, pad_count);
	while (temp_data) {
		if (temp_data->id > pad_id) {
			temp_data->prev->next = pad_data;
			pad_data->prev = temp_data->prev;
			pad_data->next = temp_data;
			temp_data->prev = pad_data;
			cisd->pad_count++;
			return;
		}
		temp_data = temp_data->next;
	}

	pr_info("%s: failed to add pad_data(%d, %d)\n",
		__func__, pad_id, pad_count);
	kfree(pad_data);
}

void init_cisd_pad_data(struct cisd* cisd)
{
	struct pad_data* temp_data = cisd->pad_array;

	mutex_lock(&cisd->padlock);
	while (temp_data) {
		struct pad_data* next_data = temp_data->next;

		kfree(temp_data);
		temp_data = next_data;
	}

	/* create dummy data */
	cisd->pad_array = create_pad_data(0, 0);
	if (cisd->pad_array == NULL)
		goto err_create_dummy_data;
	temp_data = create_pad_data(MAX_PAD_ID, 0);
	if (temp_data == NULL) {
		kfree(cisd->pad_array);
		cisd->pad_array = NULL;
		goto err_create_dummy_data;
	}
	cisd->pad_count = 0;
	cisd->pad_array->next = temp_data;
	temp_data->prev = cisd->pad_array;

err_create_dummy_data:
	mutex_unlock(&cisd->padlock);
}

void count_cisd_pad_data(struct cisd* cisd, unsigned int pad_id)
{
	struct pad_data* pad_data;

	if (cisd->pad_array == NULL) {
		pr_info("%s: can't update the connected count of pad_id(0x%x) because of null\n",
			__func__, pad_id);
		return;
	}

	mutex_lock(&cisd->padlock);
	if ((pad_data = find_pad_data_by_id(cisd, pad_id)) != NULL)
		pad_data->count++;
	else
		add_pad_data(cisd, pad_id, 1);
	mutex_unlock(&cisd->padlock);
}

static unsigned int convert_wc_index_to_pad_id(unsigned int wc_index)
{
	switch (wc_index) {
	case WC_SNGL_NOBLE:
		return WC_PAD_ID_SNGL_NOBLE;
	case WC_SNGL_VEHICLE:
		return WC_PAD_ID_SNGL_VEHICLE;
	case WC_SNGL_MINI:
		return WC_PAD_ID_SNGL_MINI;
	case WC_SNGL_ZERO:
		return WC_PAD_ID_SNGL_ZERO;
	case WC_SNGL_DREAM:
		return WC_PAD_ID_SNGL_DREAM;
	case WC_STAND_HERO:
		return WC_PAD_ID_STAND_HERO;
	case WC_STAND_DREAM:
		return WC_PAD_ID_STAND_DREAM;
	case WC_EXT_PACK:
		return WC_PAD_ID_EXT_BATT_PACK;
	case WC_EXT_PACK_TA:
		return WC_PAD_ID_EXT_BATT_PACK_TA;
	default:
		break;
	}

	return 0;
}

void set_cisd_pad_data(struct sec_battery_info *battery, const char* buf)
{
	struct cisd* pcisd = &battery->cisd;
	unsigned int pad_index, pad_total_count, pad_id, pad_count;
	struct pad_data* pad_data;
	int i, x;

	pr_info("%s: %s\n", __func__, buf);
	if (sscanf(buf, "%10d%n", &pad_index, &x) <= 0) {
		pr_info("%s: failed to read pad index\n", __func__);
		return;
	}
	buf += (size_t)x;
	pr_info("%s: stored pad_index(%d)\n", __func__, pad_index);

	if (pcisd->pad_count > 0)
		init_cisd_pad_data(pcisd);

	if (pcisd->pad_array == NULL) {
		pr_info("%s: can't set the pad data because of null\n", __func__);
		return;
	}

	if (!pad_index) {
		for (i = WC_DATA_INDEX + 1; i < WC_DATA_MAX; i++) {
			sscanf(buf, "%10d%n", &pad_count, &x);
			buf += (size_t)x;

			if (pad_count > 0) {
				pad_id = convert_wc_index_to_pad_id(i);

				mutex_lock(&pcisd->padlock);
				if ((pad_data = find_pad_data_by_id(pcisd, pad_id)) != NULL)
					pad_data->count = pad_count;
				else
					add_pad_data(pcisd, pad_id, pad_count);
				mutex_unlock(&pcisd->padlock);
			}
		}
	} else {
		sscanf(buf + 1, "%10d%n", &pad_total_count, &x);
		if (pad_total_count >= MAX_PAD_ID)
			return;
		buf += (size_t)(x + 1);

		pr_info("%s: add pad data(count: %d)\n", __func__, pad_total_count);
		for (i = 0; i < pad_total_count; i++) {
			if (sscanf(buf, " 0x%02x:%10d%n", &pad_id, &pad_count, &x) != 2) {
				pr_info("%s: failed to read pad data(0x%x, %d, %d)!!!re-init pad data\n",
					__func__, pad_id, pad_count, x);
				init_cisd_pad_data(pcisd);
				break;
			}
			buf += (size_t)x;

			mutex_lock(&pcisd->padlock);
			if ((pad_data = find_pad_data_by_id(pcisd, pad_id)) != NULL)
				pad_data->count = pad_count;
			else
				add_pad_data(pcisd, pad_id, pad_count);
			mutex_unlock(&pcisd->padlock);
		}
	}
}
