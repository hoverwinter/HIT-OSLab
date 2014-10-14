#!/usr/bin/python
import sys
import copy

P_NULL = 0
P_NEW = 1
P_READY = 2
P_RUNNING = 4
P_WAITING = 8
P_EXIT = 16

S_STATE = 0
S_TIME = 1

HZ = 100

graph_title = r"""
-----===< COOL GRAPHIC OF SCHEDULER >===-----

             [Symbol]   [Meaning]
         ~~~~~~~~~~~~~~~~~~~~~~~~~~~
             number   PID or tick
              "-"     New or Exit 
              "#"       Running
              "|"        Ready
              ":"       Waiting
                    / Running with 
              "+" -|     Ready 
                    \and/or Waiting

-----===< !!!!!!!!!!!!!!!!!!!!!!!!! >===-----
"""

usage = """
Usage: 
%s /path/to/process.log [PID1] [PID2] ... [-x PID1 [PID2] ... ] [-m] [-g]

Example:
# Include process 6, 7, 8 and 9 in statistics only. (Unit: tick)
%s /path/to/process.log 6 7 8 9

# Exclude process 0 and 1 from statistics. (Unit: tick)
%s /path/to/process.log -x 0 1

# Include process 6 and 7 only and print a COOL "graphic"! (Unit: millisecond)
%s /path/to/process.log 6 7 -m -g

# Include all processes and print a COOL "graphic"! (Unit: tick)
%s /path/to/process.log -g
"""

class MyError(Exception):
    pass

class DuplicateNew(MyError):
    def __init__(self, pid):
	    args = "More than one 'N' for process %d." % pid
	    MyError.__init__(self, args)

class UnknownState(MyError):
    def __init__(self, state):
	    args = "Unknown state '%s' found." % state
	    MyError.__init__(self, args)

class BadTime(MyError):
    def __init__(self, time):
	    args = "The time '%d' is bad. It should >= previous line's time." % time
	    MyError.__init__(self, args)

class TaskHasExited(MyError):
    def __init__(self, state):
	    args = "The process has exited. Why it enter '%s' state again?" % state
	    MyError.__init__(self, args)

class BadFormat(MyError):
    def __init__(self):
	    args = "Bad log format"
	    MyError.__init__(self, args)

class RepeatState(MyError):
	def __init__(self, pid):
		args = "Previous state of process %d is identical with this line." % (pid)
		MyError.__init__(self, args)

class SameLine(MyError):
	def __init__(self):
		args = "It is a clone of previous line."
		MyError.__init__(self, args)

class NoNew(MyError):
	def __init__(self, pid, state):
		args = "The first state of process %d is '%s'. Why not 'N'?" % (pid, state)
		MyError.__init__(self, args)

class statistics:
	def __init__(self, pool, include, exclude):
		if include:
			self.pool = process_pool()
			for process in pool:
				if process.getpid() in include:
					self.pool.add(process)
		else:
			self.pool = copy.copy(pool)

		if exclude:
			for pid in exclude:
				if self.pool.get_process(pid):
					self.pool.remove(pid)
	
	def list_pid(self):
		l = []
		for process in self.pool:
			l.append(process.getpid())
		return l

	def average_turnaround(self):
		if len(self.pool) == 0:
			return 0
		sum = 0
		for process in self.pool:
			sum += process.turnaround_time()
		return float(sum) / len(self.pool)

	def average_waiting(self):
		if len(self.pool) == 0:
			return 0
		sum = 0
		for process in self.pool:
			sum += process.waiting_time()
		return float(sum) / len(self.pool)
	
	def begin_time(self):
		begin = 0xEFFFFF
		for p in self.pool:
			if p.begin_time() < begin:
				begin = p.begin_time()
		return begin

	def end_time(self):
		end = 0
		for p in self.pool:
			if p.end_time() > end:
				end = p.end_time()
		return end

	def throughput(self):
		return len(self.pool) * HZ / float(self.end_time() - self.begin_time())

	def print_graphic(self):
		begin = self.begin_time()
		end = self.end_time()

		print graph_title

		for i in range(begin, end+1):
			line = "%5d " % i
			for p in self.pool:
				state = p.get_state(i)
				if state & P_NEW:
					line += "-"
				elif state == P_READY or state == P_READY | P_WAITING:
					line += "|"
				elif state == P_RUNNING:
					line += "#"
				elif state == P_WAITING:
					line += ":"
				elif state & P_EXIT:
					line += "-"
				elif state == P_NULL:
					line += " "
				elif state & P_RUNNING:
					line += "+"
				else:
					assert False
				if p.get_state(i-1) != state and state != P_NULL:
					line += "%-3d" % p.getpid()
				else:
					line += "   "
			print line

class process_pool:
	def __init__(self):
		self.list = []
	
	def get_process(self, pid):
		for process in self.list:
			if process.getpid() == pid:
				return process
		return None

	def remove(self, pid):
		for process in self.list:
			if process.getpid() == pid:
				self.list.remove(process)

	def new(self, pid, time):
		p = self.get_process(pid)
		if p:
			if pid != 0:
				raise DuplicateNew(pid)
			else:
				p.states=[(P_NEW, time)]
		else:
			p = process(pid, time)
			self.list.append(p)
		return p

	def add(self, p):
		self.list.append(p)

	def __len__(self):
		return len(self.list)
	
	def __iter__(self):
		return iter(self.list)

class process:
	def __init__(self, pid, time):
		self.pid = pid
		self.states = [(P_NEW, time)]
	
	def getpid(self):
		return self.pid

	def change_state(self, state, time):
		last_state, last_time = self.states[-1]
		if state == P_NEW:
			raise DuplicateNew(pid)
		if time < last_time:
			raise BadTime(time)
		if last_state == P_EXIT:
			raise TaskHasExited(state)
		if last_state == state and self.pid != 0: # task 0 can have duplicate state
			raise RepeatState(self.pid)

		self.states.append((state, time))

	def get_state(self, time):
		rval = P_NULL
		combo = P_NULL
		if self.begin_time() <= time <= self.end_time():
			for state, s_time in self.states:
				if s_time < time:
					rval = state
				elif s_time == time:
					combo |= state
				else:
					break
			if combo:
				rval = combo
		return rval

	def turnaround_time(self):
		return self.states[-1][S_TIME] - self.states[0][S_TIME]

	def waiting_time(self):
		return self.state_last_time(P_READY)

	def cpu_time(self):
		return self.state_last_time(P_RUNNING)

	def io_time(self):
		return self.state_last_time(P_WAITING)

	def state_last_time(self, state):
		time = 0
		state_begin = 0
		for s,t in self.states:
			if s == state:
				state_begin = t
			elif state_begin != 0:
				assert state_begin <= t
				time += t - state_begin
				state_begin = 0
		return time


	def begin_time(self):
		return self.states[0][S_TIME]

	def end_time(self):
		return self.states[-1][S_TIME]
		
# Enter point
if len(sys.argv) < 2:
	print usage.replace("%s", sys.argv[0])
	sys.exit(0)

# parse arguments
include = []
exclude = []
unit_ms = False
graphic = False
ex_mark = False

try:
	for arg in sys.argv[2:]:
		if arg == '-m':
			unit_ms = True
			continue
		if arg == '-g':
			graphic = True
			continue
		if not ex_mark:
			if arg == '-x':
				ex_mark = True
			else:
				include.append(int(arg))
		else:
			exclude.append(int(arg))
except ValueError:
	print "Bad argument '%s'" % arg
	sys.exit(-1)

# parse log file and construct processes
processes = process_pool()

f = open(sys.argv[1], "r")

# Patch process 0's New & Run state
processes.new(0, 40).change_state(P_RUNNING, 40)

try:
	prev_time = 0
	prev_line = ""
	for lineno, line in enumerate(f):

		if line == prev_line:
			raise SameLine
		prev_line = line

		fields = line.split("\t")
		if len(fields) != 3:
			raise BadFormat

		pid = int(fields[0])
		s = fields[1].upper()

		time = int(fields[2])
		if time < prev_time:
			raise BadTime(time)
		prev_time = time

		p = processes.get_process(pid)

		state = P_NULL
		if s == 'N':
			processes.new(pid, time)
		elif s == 'J':
			state = P_READY
		elif s == 'R':
			state = P_RUNNING
		elif s == 'W':
			state = P_WAITING
		elif s == 'E':
			state = P_EXIT
		else:
			raise UnknownState(s)
		if state != P_NULL:
			if not p:
				raise NoNew(pid, s)
			p.change_state(state, time)
except MyError, err:
	print "Error at line %d: %s" % (lineno+1, err)
	sys.exit(0)

# Stats
stats = statistics(processes, include, exclude)
att = stats.average_turnaround()
awt = stats.average_waiting()
if unit_ms:
	unit = "ms"
	att *= 1000/HZ
	awt *= 1000/HZ
else:
	unit = "tick"
print "(Unit: %s)" % unit
print "Process   Turnaround   Waiting   CPU Burst   I/O Burst"
for pid in stats.list_pid():
	p = processes.get_process(pid)
	tt = p.turnaround_time()
	wt = p.waiting_time()
	cpu = p.cpu_time()
	io = p.io_time()

	if unit_ms:
		print "%7d   %10d   %7d   %9d   %9d" % (pid, tt*1000/HZ, wt*1000/HZ, cpu*1000/HZ, io*1000/HZ)
	else:
		print "%7d   %10d   %7d   %9d   %9d" % (pid, tt, wt, cpu, io)
print "Average:  %10.2f   %7.2f" % (att, awt)
print "Throughout: %.2f/s" % (stats.throughput())

if graphic:
	stats.print_graphic()
