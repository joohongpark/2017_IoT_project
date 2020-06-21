#!/usr/bin/python
import sqlite3
import random
import time
import types

def SQL_INPUT_DATA(sql_obj,dat_time,dat_id,dat_energy):
	c = sql_obj.execute('SELECT energy,index_data FROM POWER_INT WHERE time = ? AND id = ?', (dat_time,dat_id)).fetchone()
	if type(c) == types.NoneType:
		sql_obj.execute('INSERT INTO POWER_INT VALUES (NULL,?,?,?)', (dat_id,dat_time,0))
		sql_obj.commit()
	else:
		sql_obj.execute('UPDATE POWER_INT SET energy = ? WHERE index_data = ?', (c[0] + dat_energy,c[1]))
		sql_obj.commit()

conn = sqlite3.connect("dbdb.db")
now_hour = time.strftime("%y%m%d%H")

for i in range(1,22):
	SQL_INPUT_DATA(conn,now_hour,i,int('12'))
conn.close()