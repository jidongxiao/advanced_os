# Lab 3: Exploring Process Scheduling with `nice`

## Objective

In this lab, you will use a provided CPU-intensive Bash script to investigate how the Linux scheduler treats processes with different **nice values**.

You will:

* Run a CPU-bound process.
* Observe its nice value and CPU usage.
* Run multiple CPU-bound processes simultaneously.
* Change the nice value of a running process.
* Observe how different nice values affect CPU usage.

---

## Background

Linux uses a **nice value** to influence the scheduling of CPU-bound processes.

The normal nice value is:

```text
NI = 0
```

Nice values range from:

```text
-20  ← higher scheduling priority
  0  ← normal
+19  ← lower scheduling priority
```

A lower nice value gives a process a greater scheduling preference, while a higher nice value gives it a lower scheduling preference.

Importantly, the nice value does **not** specify a fixed percentage of CPU time. It affects how CPU time is distributed when processes compete for CPU resources.

---

## Provided Program

You are provided with the bash script [cpu_stress.sh](cpu_stress.sh):

```text
cpu_stress.sh
```

The script performs continuous arithmetic calculations for a specified amount of time. It is designed to create a **CPU-bound process**.

You do not need to modify the script.

Make sure it is executable:

```bash
chmod +x cpu_stress.sh
```

You can test it by running:

```bash
./cpu_stress.sh 10
```

The argument specifies how many seconds the program should run.

---

# Part 1: Observe a CPU-Bound Process

Start the CPU-intensive program for 60 seconds in the background:

```bash
./cpu_stress.sh 60 &
```

The shell will display a PID similar to:

```text
[1] 12345
```

Record the PID.

Now examine the process:

```bash
ps -p PID -o pid,ni,pcpu,stat,comm
```

Replace `PID` with the actual process ID.

For example:

```bash
ps -p 12345 -o pid,ni,pcpu,stat,comm
```

You should see something similar to:

```text
    PID  NI %CPU STAT COMMAND
  12345   0 98.5 R    cpu_stress.sh
```

### Question 1

What is the process's initial nice value?

### Question 2

What does the `%CPU` value tell you about this process?

---

# Part 2: Run Two CPU-Bound Processes

Start two instances of the program:

```bash
taskset -c 0 ./cpu_stress.sh 60 &
taskset -c 0 ./cpu_stress.sh 60 &
```

Find both processes:

```bash
ps -eo pid,ni,pcpu,stat,comm | grep cpu_stress
```

You should see something similar to:

```text
  PID   NI %CPU STAT COMMAND
12345    0 49.5 R    cpu_stress.sh
12346    0 49.3 R    cpu_stress.sh
```

The exact CPU percentages will depend on your machine.

### Question 3

How is the CPU time divided between the two processes?

Do not worry if the values are not exactly equal.

---

# Part 3: Change the Nice Value

Start two new CPU-bound processes:

```bash
taskset -c 0 ./cpu_stress.sh 90 &
PID1=$!

taskset -c 0 ./cpu_stress.sh 90 &
PID2=$!
```

Verify their nice values:

```bash
ps -p $PID1,$PID2 -o pid,ni,pcpu,stat,comm
```

Both processes should initially have:

```text
NI = 0
```

Now change the nice value of the second process:

```bash
renice 10 -p $PID2
```

Verify the change:

```bash
ps -p $PID1,$PID2 -o pid,ni,pcpu,stat,comm
```

Wait a few seconds and run the command again:

```bash
ps -p $PID1,$PID2 -o pid,ni,pcpu,stat,comm
```

Take a screenshot (Screenshot 1) of this command and its output, making sure that both processes, their nice values, and their CPU usage are clearly visible.

### Question 4

Which process receives more CPU time?

---

# Part 4: Increase the Difference

Start two more CPU-bound processes:

```bash
taskset -c 0 ./cpu_stress.sh 90 &
PID3=$!

taskset -c 0 ./cpu_stress.sh 90 &
PID4=$!
```

Change the second process to a much higher nice value:

```bash
renice 19 -p $PID4
```

Check the processes:

```bash
ps -p $PID3,$PID4 -o pid,ni,pcpu,stat,comm
```

Wait several seconds and check again.

```bash
ps -p $PID3,$PID4 -o pid,ni,pcpu,stat,comm
```

Take a screenshot (Screenshot 2) of this command and its output, making sure that both processes, their nice values, and their CPU usage are clearly visible.

### Question 5

Compare this experiment with Part 3.

What happens when the difference between the nice values becomes larger?

---

# Important Observation

The experiments demonstrate a **relative scheduling preference**:

```text
Lower nice value
       │
       ▼
Greater scheduling preference
       │
       ▼
Potentially more CPU time
       │
       ▼
When competing with other runnable processes
```

Conversely:

```text
Higher nice value
       │
       ▼
Lower scheduling preference
       │
       ▼
Potentially less CPU time
```

The effect is most apparent when CPU-bound processes are **competing for the same CPU resources**.

---

# Cleanup

The stress programs automatically terminate when their requested runtime expires.

If you need to terminate a process early:

```bash
kill PID
```

For example:

```bash
kill $PID5
```

You can check whether any remain:

```bash
ps -eo pid,ni,pcpu,stat,comm | grep cpu_stress
```

---

# Summary

In this lab, you experimentally observed that:

1. A CPU-bound process continuously competes for CPU resources.
2. Processes normally start with `NI = 0`.
3. `renice` can change the nice value of a running process.
4. A lower nice value gives a process greater scheduling preference.
5. A higher nice value gives a process lower scheduling preference.
6. Nice values affect **relative CPU scheduling**, not a fixed CPU percentage.

# Submission

Submit the following three files on Submitty:

Screenshot 1 — Your observation from the experiment in part 3.
Screenshot 2 — Your observation from the experiment in part 4.
TXT file — Your answers to the five lab questions.

Make sure that both screenshots clearly show the relevant command and its output.

Due Date: 09/21/2026, 11:59pm. Each lab has a maximum of 5 points. Late submissions will be accepted within 3 days after the deadline, with a 1-point penalty applied to the earned grade.
