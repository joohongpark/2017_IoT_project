#!/usr/bin/python
#coding=utf-8
'''
라즈베리파이에서 시리얼 포트에 접근헤 데이터를 받아와 파싱하여 SQLite DB에 저장함.
해당 코드는 라즈베리파이에서 동작됨.
SQLite DB 접근을 위해 WebApp 폴더 상에서 동작되게 코딩되어 있다.
'''
import serial
import json
import threading
import sqlite3
import random
import time
import types

CONST = []
conn = sqlite3.connect("dbdb.db")
kk = conn.execute('SELECT id, A, B FROM "PARAM" WHERE 1')
if type(kk) != types.NoneType:
	for row in kk:
		CONST.append([float(row[1]), float(row[2])])
conn.close()
print CONST

line = []
count = [[0 for col in range(10)] for row in range(24)]
str_line = ""
DEV_PATH = '/dev/ttyUSB0'

def TIMER_MAN(sql_obj):
	s = sql_obj.execute('SELECT id, ctrl FROM TIMER_CTRL WHERE ctrl = 1')
	h = time.strftime("%H")
	m = time.strftime("%M")
	onoff_arr = []
	i = 0

	t = 60 * int(h) + int(m)
	if type(s) != types.NoneType:
		for row in s:
			onoff_arr.append([0, row[0]])
			tt = sql_obj.execute('SELECT id, start_time, stop_time FROM TIMER WHERE id = ?', [row[0]])
			for l in tt:
				if type(l) != types.NoneType:
					if(l[1] < l[2]):
						print "1"
						if (l[1] < t) & (t < l[2]):
							onoff_arr[i][0] = 1
					else:
						if (l[1] < t) | (t < l[2]):
							onoff_arr[i][0] = 1
					print( str(l[0]) + " -> " + str(l[1]) + " ~ " + str(l[2]) + " , " + str(t))
	#		sql_obj.execute('UPDATE ONOFF SET onoff = ? WHERE id = ?', (c[0] + dat_energy,c[1]))
	#		sql_obj.commit()
			i = i + 1
	print onoff_arr
	sql_obj.executemany('UPDATE ONOFF SET onoff = ? WHERE id = ?', onoff_arr)
	sql_obj.commit()

def SERIAL_COMM(sql_obj, serial_obj):
	s = sql_obj.execute('SELECT id, onoff FROM ONOFF WHERE id IN (SELECT id FROM IS_EXISTS WHERE exist = 1)')
	if type(s) != types.NoneType:
		for row in s:
			serial_obj.writelines('c' + (('0' + str(row[0])) if row[0] < 10 else str(row[0]) ) + str(row[1]) + chr(0x0D))

def SQL_INPUT_ENERGY_DATA(sql_obj,dat_time,dat_id,dat_energy):
	k = 0
	try:
		c = sql_obj.execute('SELECT energy,index_data FROM POWER_INT WHERE time_data = ? AND id = ?', (dat_time,dat_id)).fetchone()
		if type(c) == types.NoneType:
			sql_obj.execute('INSERT INTO POWER_INT VALUES (NULL,?,?,?)', (dat_id,dat_time,0))
			sql_obj.commit()
		else:
			sql_obj.execute('UPDATE POWER_INT SET energy = ? WHERE index_data = ?', (c[0] + dat_energy,c[1]))
			sql_obj.commit()
			k = c[0] + dat_energy
	except ValueError:
		print ValueError
	return k


def READ_Serial(ser):
	# 시리얼 데이터 파싱
    global line
    while True:
		for c in ser.read():
			line.append(c)
			if c == '\r':
				str_line = "".join(line)
				str_line = str_line.strip("\r\n")
				PARSE_JSON(str_line, ser)
				del line[:]

def return_watt(E, P, K):
	# ADE7753의 전력 특정 RAW Data를 실제 전력 단위 W로 변환시켜줌.
	if not E == 0 and not P == 0:
		CLKIN = 3686400
		LINECYC = 240
		K = 4.322716741E-05
		return (K * E * CLKIN * 900)/(LINECYC * P)
	else:
		return 0
def PARSE_JSON(strs, serial_obj):
	# 시리얼로부터 수신된 JSON 형식의 데이터를 문자열로 받아 파싱하여 DB에 저장 또는 업데이트
	global count
	conn = sqlite3.connect("dbdb.db")
	now_hour = time.strftime("%y%m%d%H")
	SERIAL_COMM(conn, serial_obj)
	onoff = []
	for row in range(24):
		onoff.append([0, row])
	try:
		cc = json.loads(strs)
		for j in range(0,23):
			count[j].pop(0)
			count[j].append(0)
		for i in cc:
			count[int(i)][9] = 1
			t1 = return_watt( int(cc[i]['RAENERGY']) + CONST[int(i) - 1][1], int(cc[i]['PERIOD']), CONST[int(i) - 1][0])
			t2 = t1 / 3600
			if int(cc[i]['RAENERGY']) > -50:
				t3 = SQL_INPUT_ENERGY_DATA(conn,now_hour, i, t2)
				print("id%d : watt : %dW , energy : %dWH" % ( int(i), t1, t2))
				if int(cc[i]['PERIOD']):
					conn.execute('UPDATE POWER_NOW SET energy = ?, pow = ? WHERE id = ?', [t3, t1 ,int(i)])
					conn.commit()
		for j in range(0,23):
			if sum(count[j]) >= 1:
				onoff[j][0] = 1
		conn.executemany('UPDATE IS_EXISTS SET exist = ? WHERE id = ?', onoff)
		conn.commit()
	except ValueError:
		print ValueError
	conn.close()

def man_callback():
	# 파이썬 스레딩관련 콜백함수
	timer = threading.Timer(10, man_callback)
	conn = sqlite3.connect("dbdb.db")
	TIMER_MAN(conn)
	timer.start()

try:
	comm = serial.Serial(DEV_PATH, baudrate = 115200)
except:
	print "시리얼 접속 에러"
	exit()
thread = threading.Thread(target=READ_Serial, args=(comm,))
thread.start()
man_callback()