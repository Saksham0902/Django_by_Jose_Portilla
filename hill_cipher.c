
#include <stdio.h>

#include "commons.h"
#include "ciphers.h"


#define MATRIX_SIZE 3


#define BASE_MOD 26


#define PAD_NULL 'x'

char hc_key_matrix[MATRIX_SIZE][MATRIX_SIZE];


void hc_populate_key(string key) {

	unsigned int string_length = strlen(key);
	unsigned int counter = 0;
	for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++)
		hc_key_matrix[i / MATRIX_SIZE][i % MATRIX_SIZE] =
			(i < string_length) ? key[i] : (char) counter++ + 97;
}

int mod(int a, int b) {
	int r = a % b;
	return r < 0 ? r + b : r;
}


void hc_populate_inverse(string key) {
	// Generating the key matrix for the algorithm - this matrix will then
	// be inverted for the inverse matrix.
	hc_populate_key(key);

	// Creating a temporary matrix as the augmented matrix.
	int augmented_matrix[MATRIX_SIZE][MATRIX_SIZE];

	float determinant = 0;
	for (int i = 0; i < 3; i++)
		determinant = determinant + (
			(hc_key_matrix[0][i] - 97) * (
				(hc_key_matrix[1][(i + 1) % 3] - 97) *
				(hc_key_matrix[2][(i + 2) % 3] - 97) -
				(hc_key_matrix[1][(i + 2) % 3] - 97) *
				(hc_key_matrix[2][(i + 1) % 3] - 97)
			)
		);

	// Getting the result and the multiplicative inverse at once.
	int result = (int) determinant % 26;
	int multi_inverse = 0;
	while ((multi_inverse * result) % 26 != 1)
		multi_inverse++;

	// Populating augmented matrix with initial set of values - will contain
	// the co-factor matrix by the end of this step.
	bool negative = false;
	for (unsigned int row = 0; row < MATRIX_SIZE; row++) {
		for (unsigned int column = 0; column < MATRIX_SIZE; column++) {
			int first = 1;
			int second = 1;

			unsigned int first_row = 0;
			for (unsigned int i = 0; i < MATRIX_SIZE; i++)
				for (unsigned int j = 0; j < MATRIX_SIZE; j++)
					if (i != row && j != column) {
						if (first_row == 0 || first_row == 3)
							first *= hc_key_matrix[i][j] - 97;
						else
							second *= hc_key_matrix[i][j] - 97;

						first_row++;
					}

			augmented_matrix[row][column] = ((negative) ? -1 : 1) * (first - second);
			negative = !negative;
		}
	}

	// Converting the co-factor matrix into transpose - to get adjoint matrix.
	for (unsigned int i = 0; i < MATRIX_SIZE; i++)
		for (unsigned int j = 0; j < MATRIX_SIZE; j++)
			if (i <= j) {
				int temp = mod(augmented_matrix[i][j], 26);

				augmented_matrix[i][j] = mod(augmented_matrix[j][i], 26);
				augmented_matrix[j][i] = temp;
			}

	// Multiplying by the multiplicative inverse, and storing the result in the
	// original matrix, from where it will be used to get the result.
	for (unsigned int i = 0; i < MATRIX_SIZE; i++)
		for (unsigned int j = 0; j < MATRIX_SIZE; j++)
			hc_key_matrix[i][j] = mod(augmented_matrix[i][j] * multi_inverse, 26) + 97;
}


void _hc_print_key(string pad_char, string end_line) {
	for (unsigned int i = 0; i < MATRIX_SIZE; i++) {
		// Avoiding printing a new line before the start of the matrix. While
		// ensuring that the first line is actually padded with the character.
		printf("%c%s", (i != 0) ? '\n' : '\0', pad_char);
		for (unsigned int j = 0; j < MATRIX_SIZE; j++)
			printf("%c  ", hc_key_matrix[i][j]);
	}

	// Printing the end-line character.
	printf("%s", end_line);
}

/**
 * Just a convenience method to be able to print the 2d matrix as needed. Exists
 * to complement the debugger, and have a graphical representation of the ciphers.
 *

 */
extern inline void hc_print_key() {
	_hc_print_key("", "\n");
}



extern inline unsigned int map(char c) {
	if ((int) c < 97) {
		// Raise an error if an invalid mapping is attempted - reduces the possibility
		// of logical bugs, instead converts them into runtime errors.
		printf("Error: Attempt to map character `%c` in Hill Cipher.\n", c);
		exit(-10);
	}

	return (int) c - 97;
}



extern inline char rev_map(int i) {
	if (i > 26) {
		// Raise an error if an invalid mapping is attempted - reduces the possibility
		// of logical bugs, instead converts them into runtime errors.
		printf("Error: Attempt to reverse map integer `%d` in Hill Cipher.\n", i);
		exit(-10);
	}

	return (char) i + 97;
}


void hc_current_mapping(string multiplier, string result, const_str padding, const_str end_line) {
	bool mid_line_found = false;
	for (unsigned int i = 0; i < MATRIX_SIZE; i++) {
		// Printing on a new line if this isn't the first row of the matrix.
		printf("%c%s", (i == 0) ? '\0' : '\n', padding);
		for (unsigned int j = 0; j < MATRIX_SIZE; j++)
			printf(
				// The first character will be the alphabet being multiplied, and the string following it
				// will be the padding as needed (removed in case of last column).
				"%c%s",
				hc_key_matrix[i][j],
				(j + 1 == MATRIX_SIZE) ? "" : "  "
			);

		// Separating the matrices with space, printing a multiplication/equality
		// sign between the two if the current line is a mid-line.
		if (i >= MATRIX_SIZE / 2 && !mid_line_found) {
			mid_line_found = true;
			printf("%s%c%s%c", "   x   ", multiplier[i], "   =   ", result[i]);
		} else {
			printf("%s%c%s%c", "       ", multiplier[i], "       ", result[i]);
		}
	}

	printf("%s", end_line);
}



string crypt_hill_cipher(string message, string key, bool verbose) {
	// Generating the key matrix.
	hc_populate_key(key);

	if (verbose) {
		printf("\nKey Matrix:\n");
		_hc_print_key("\t", "\n\n");
		printf("Original Message: \n\t`%s`\n", message);
	}

	// Temporary string(s) to hold `n` characters in the string at the time.
	string temp = (string) malloc(MATRIX_SIZE * sizeof(char));
	string temp_result = (string) malloc(MATRIX_SIZE * sizeof(char));

	// Calculating the length of the original message, and of the result - they might
	// not always match. If the message length is not a multiple of `MATRIX_SIZE`, it
	// will be padded with extra characters to make it fit.
	unsigned int message_length = strlen(message);
	unsigned int result_length = message_length + (message_length % MATRIX_SIZE);

	// The result string - will be used to contain the result as it is being generated.
	string result = (string) malloc(result_length * sizeof(char));

	// Starting a loop to iterate between every `MATRIX_SIZE` elements. If a
	// tri-graph is selected for example, iterating between every three elements.
	for (unsigned int i = 0; i < result_length; i += MATRIX_SIZE) {
		for (unsigned int counter = 0; counter < MATRIX_SIZE; counter++)
			// Picking up the first `n` characters from the current position - if the
			// message has ran out of characters, padding with null character.
			temp[counter] = (i + counter < message_length) ? message[i + counter] : PAD_NULL;

		// Matrix multiplication - treat the contents of the temp string as a matrix, and perform
		// multiplication with the key matrix.
		for (unsigned int j = 0; j < MATRIX_SIZE; j++) {
			unsigned int val = 0;

			for (unsigned int k = 0; k < MATRIX_SIZE; k++)
				val += map(hc_key_matrix[j][k]) * map(temp[k]);

			// Adding the results to the temp result string - to make sure that the contents of this
			// multiplication can be printed in verbose mode. Calculating the modulus using the macro.
			temp_result[j] = rev_map(val % BASE_MOD);
		}

		if (verbose) {
			printf("\n\nIteration %d:\n", (i / 3) + 1);
			strcat(result, temp_result);

			hc_current_mapping(
				temp,
				temp_result,
				"\t",
				"\n\n"
			);

			printf("Current Result: \n\t`%s`\n", result);
		}
	}

	// Freeing up space from the temporary string - just a good practice.
	free(temp);

	return gen_str(result);
}

string decrypt_hill_cipher(string message, string key, bool verbose) {
	// Generating the key matrix.
	hc_populate_inverse(key);

	if (verbose) {
		printf("\nKey Matrix:\n");
		_hc_print_key("\t", "\n\n");
		printf("Original Message: \n\t`%s`\n", message);
	}

	// Temporary string(s) to hold `n` characters in the string at the time.
	string temp = (string) malloc(MATRIX_SIZE * sizeof(char));
	string temp_result = (string) malloc(MATRIX_SIZE * sizeof(char));

	// Calculating the length of the original message, and of the result - they might
	// not always match. If the message length is not a multiple of `MATRIX_SIZE`, it
	// will be padded with extra characters to make it fit.
	unsigned int message_length = strlen(message);
	unsigned int result_length = message_length + (message_length % MATRIX_SIZE);

	// The result string - will be used to contain the result as it is being generated.
	string result = (string) malloc(result_length * sizeof(char));

	// Starting a loop to iterate between every `MATRIX_SIZE` elements. If a
	// tri-graph is selected for example, iterating between every three elements.
	for (unsigned int i = 0; i < result_length; i += MATRIX_SIZE) {
		for (unsigned int counter = 0; counter < MATRIX_SIZE; counter++)
			// Picking up the first `n` characters from the current position - if the
			// message has ran out of characters, padding with null character.
			temp[counter] = (i + counter < message_length) ? message[i + counter] : PAD_NULL;

		// Matrix multiplication - treat the contents of the temp string as a matrix, and perform
		// multiplication with the key matrix.
		for (unsigned int j = 0; j < MATRIX_SIZE; j++) {
			unsigned int val = 0;

			for (unsigned int k = 0; k < MATRIX_SIZE; k++)
				val += map(hc_key_matrix[j][k]) * map(temp[k]);

			// Adding the results to the temp result string - to make sure that the contents of this
			// multiplication can be printed in verbose mode. Calculating the modulus using the macro.
			temp_result[j] = rev_map(val % BASE_MOD);
		}

		if (verbose) {
			printf("\n\nIteration %d:\n", (i / 3) + 1);
			strcat(result, temp_result);

			hc_current_mapping(
				temp,
				temp_result,
				"\t",
				"\n\n"
			);

			printf("Intermediate Result: \n\t`%s`\n", result);
		}
	}

	// Freeing up space from the temporary string - just a good practice.
	free(temp);

	return gen_str(result);
}
