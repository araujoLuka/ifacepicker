# ifacepicker

`ifacepicker` is a simple C++ program that facilitates the viewing of network interfaces and their IP addresses, providing an easy selection process. 
It utilizes the 'ip a' command to retrieve information about network interfaces.

## Purpose

The purpose of `ifacepicker` is to assist in resolving the challenge of selecting a specific network interface or IP address in scripting scenarios. 
It aims to enhance the visibility of the machine's IP addresses on different interfaces. Notably, it played a helpful role in configuring Wake-on-LAN in another personal project.

## Compilation

To compile the program, use the following command:

```bash
g++ main.cpp -o ifacepicker
```

Or use `make`.

## Usage

Run the compiled program without any arguments to display a list of network interfaces. Follow the on-screen prompts to select an interface for IP configuration.

```bash
./ifacepicker
```

## Author

Lucas Araujo - 2023-12-15

## License

`ifacepicker` is released under the [Unlicense](https://unlicense.org) license. See the [LICENSE](LICENSE) file for details.
