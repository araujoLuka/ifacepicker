# ifacepicker

`ifacepicker` is a simple program that lists network interfaces and allows users to select one for IP configuration. It utilizes the 'ip a' command to retrieve information about network interfaces and their respective IP addresses.

## Purpose

The primary objective of `ifacepicker` is to provide a convenient way to view network interfaces and choose one for IP configuration. It simplifies the process of identifying available interfaces and facilitates user-friendly IP setup.

## Compilation

To compile the program, use the following command:

```bash
g++ main.cpp -o ifacepicker
```

## Usage

Run the compiled program without any arguments to display a list of network interfaces. Follow the on-screen prompts to select an interface for IP configuration.

```bash
./ifacepicker
```

## Author

Lucas Araujo - 2023-12-15
