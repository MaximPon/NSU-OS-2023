#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <iostream>

class ShiftTable final {
	std::vector<off_t> shifts{};
	std::vector<off_t> lengths{};

public:
	off_t get_shift(int idx) {
		return shifts[idx];
	}

	off_t get_length(int idx) {
		return lengths[idx];
	}

	void push_back_shift(off_t shift) {
		shifts.push_back(shift);
	}

	void push_back_length(off_t length) {
		lengths.push_back(length);
	}
};

int main(int argc, char** argv) {
	if (argc < 2) {
		std::cerr << "No text file" << std::endl;
		return 1;
	}
	int fd = 0;
	if ((fd = open(argv[1], O_RDONLY)) == -1) {
		perror("File is not opened");
		return 0;
	}

	ShiftTable table;

	int lines_counter = 0;
	char buffer[BUFSIZ];
	off_t current_length = 0;
	off_t total_shift = 0;

	char current_char;
	size_t read_counter = 1;

	while (read_counter != 0) {
		read_counter = read(fd, buffer, sizeof(buffer));
		if (read_counter == -1) {
			perror("Read error");
			return 1;
		}
		size_t i;
		for (i = 0; i < read_counter; ++i) {
			++total_shift;
			++current_length;
			current_char = buffer[i];
			if (current_char == '\n') {
				table.push_back_length(current_length);
				table.push_back_shift(total_shift - current_length);
				current_length = 0;
				++lines_counter;
			}
		}
		table.push_back_length(current_length + 1);
		table.push_back_shift(total_shift - current_length);
	}

	if (current_char == '\n') {
		--lines_counter;
	}

	size_t line_number = 1;

	while (1) {
		std::cout << "Enter line number: ";
		std::cin >> line_number;
		if (std::cin.fail()) {
			std::cout << std::endl;
			std::cerr << "Wrong characters" << std::endl;
			std::cin.clear();
			continue;
		}
		if (line_number == 0) {
			break;
		}
		--line_number;
		if (line_number > lines_counter) {
			std::cerr << "Entered number is greater then actual" << std::endl;
			continue;
		}
		if (lseek(fd, table.get_shift(line_number), SEEK_SET) == -1) {
			perror("lseek error!");
			return 1;
		}
		off_t line_len = lines_lengths[line_number];
		off_t remain_counter = table.get_length(line_number) - 1;
		char output_buffer[BUFSIZ];
		std::cout << line_number + 1 << std::endl;
		read_counter = 1;
		while (read_counter != 0 && remain_counter > 0) {
			if (remain_counter < sizeof(output_buffer)) {
				read_counter = read(fd, output_buffer, remain_counter);
			}
			else {
				read_counter = read(fd, output_buffer, sizeof(output_buffer) - 1);
			}
			if (read_counter == -1) {
				perror("Read error!");
				return 1;
			}
			remain_counter -= read_counter;
			output_buffer[read_counter] = '\0';
			std::cout << output_buffer;
		}
		std::cout << std::endl;
	}
	return 0;
}