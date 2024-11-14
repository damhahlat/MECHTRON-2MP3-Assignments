#include "utility.h"

WordData* read_data(char *filename) {

    // Open the file
    FILE *file = fopen(filename, "r");

    // Ensure the file was successfully opened
    if (file == NULL) {
        printf("Error opening file\n");
        return NULL;
    }

    // Allocate memory for the first block of WordData
    WordData *data = malloc(sizeof(WordData));
    if (data == NULL) {
        printf("Memory allocation failed\n");
        fclose(file);
        return NULL;
    }

    int i = 0;

    // Read data from the file
    while (1) {

		// Allocate memory for the WordData item
		data = realloc(data, (i+1) * sizeof(WordData));

		// Ensure memory was allocated
		if (data == NULL) {
			printf("Memory allocation failed\n");
			free(data);
			fclose(file);
			return NULL;
		}

        // Read the word and two float values
        fscanf(file, "%s %f %f", &data[i].word, &data[i].value1, &data[i].value2);

		// Break if end of file
		if (feof(file)) {
			data[i].word[0] = '\0';
			break;
		}

        // Read the integer array
        char line[2000];
        fgets(line, 2000, file);  // Read the line for integers

        char *token = strtok(line, "[], \n\t\v\f\r");

        for (int j = 0; j < ARRAY_SIZE && token != NULL; j++) {
            data[i].intArray[j] = atoi(token);
            token = strtok(NULL, "[], \n\t\v\f\r");
        }


        // Prepare to read the next WordData item
        i++;

    }

    // Close the file
    fclose(file);

	// Return the data
	return data;
}

// Linear search for a word in the WordData array
WordData find_data(WordData *data, char *word) {

	// Search for the word in the data array
	for (int i = 0; data[i].word[0] != '\0'; i++) {
		if (strcmp(data[i].word, word) == 0) {
			return data[i];
		}
	}


	// Return a WordData item with a null word
	WordData nullData;
	nullData.word[0] = '\0';
	return nullData;
}

float calculate_sentiment_score(WordData *data, char *sentence) {

	// Initialize the scores array
	float scores[MAX_STRING_LENGTH] = { 0.0 }; // ONLY FOR TESTING PURPOSES
	int index = 0;

	// Count of all words with sentiment
	int sentimentCount = 0;

	// Sum of all sentiment scores
	float sentimentSum = 0.0;

	// Stores the current intensifier values
	float intensifier = 0;

	// Array to store the split sentence
	char sentence_split[MAX_STRING_LENGTH][MAX_STRING_LENGTH];

	// Copy the sentence for strtok to work
	char sentence_copy[MAX_STRING_LENGTH];
	strcpy(sentence_copy, sentence);

	// Since the only punctuation we care about is exclamation marks
	// We will remove all other punctuation
	char *token = strtok(sentence_copy, " \n\t\v\f\r,.?");

	// Loop through the tokens
	for (; token != NULL; index++) {

		// Booleans to check for all caps and exclamations
		bool allCaps = true;
		int exclamation = 0;

		// Convert the token to lowercase
		char lowerToken[MAX_STRING_LENGTH];
		strcpy(lowerToken, token);

		printf("Token: %s\n", lowerToken);

		// Iterate through each character and use tolower()
		for (int i = 0; lowerToken[i] != '\0'; i++) {

			// Check for all uppercase and ensure exclamation marks are not counted
			if (islower(lowerToken[i]) && lowerToken[i] != '!') allCaps = false;

			// Convert to lowercase
			lowerToken[i] = tolower(lowerToken[i]);

			// Check for exclamation marks
			if (lowerToken[i] == '!') {

				// Increment the exclamation count and null the character
				exclamation++;
				lowerToken[i] = '\0';

				// Limit the exclamation count to 3
				if (exclamation > 3) exclamation = 3;

			}
		}

		// Copy the token to the sentence split array
		strcpy(sentence_split[index], lowerToken);

		// Find the word in the data array
		WordData wordData = find_data(data, lowerToken);

		// Check if the word was found
		if (wordData.word[0] != '\0') {

			// Increment the sentiment count
			sentimentCount++;

			// Add the score to the scores array
			scores[index] = wordData.value1;

			// If word is capitalized, multiply by factor
			if (allCaps) {
				scores[index] *= CAPS;
			}

			// Add intensifier
			scores[index] += intensifier * scores[index];

			// Check negations
			if (index > 0) {
				for (int i = 0; i < NEGATIONS_SIZE; i++) {
					if (strcmp(sentence_split[index - 1], negation_words[i]) == 0) {
						scores[index] *= NEGATION;
					}
				}
			}

			// Add exclamation mark if positive, and subtract if negative
			if (scores[index] > 0) {
				scores[index] += exclamation * EXCLAMATION;
			} else {
				scores[index] -= exclamation * EXCLAMATION;
			}
			sentimentSum += scores[index];
		}

		intensifier = 0;

		// Check if current word is an intensifier
		if (index > 0) {

			// Check positive intensifiers
			for (int i = 0; i < POSITIVE_INTENSIFIERS_SIZE; i++) {
				if (strcmp(sentence_split[index - 1], positive_intensifiers[i]) == 0) {
					intensifier = INTENSIFIER;
				}
			}

			// Check negative intensifiers
			for (int i = 0; i < NEGATIVE_INTENSIFIERS_SIZE; i++) {
				if (strcmp(sentence_split[index - 1], negative_intensifiers[i]) == 0) {
					intensifier = -INTENSIFIER;
				}
			}

			// If intensifier is capitalized, multiply by factor
			if (allCaps && intensifier != 0) {
				intensifier *= CAPS;
				printf("Intensifier: %f\n", intensifier);
			}
		}

        token = strtok(NULL, " \n\t\v\f\r,.?");
	}

	// Print the words with their results
	/*for (int i = 0; i < index; i++) {*/
	/*	printf("Word: %s Score: %f\n", sentence_split[i], scores[i]);*/
	/*	token = strtok(NULL, " \n\t\v\f\r,.?");*/
	/*}*/

	float compound = sentimentSum / sqrt( pow(sentimentSum, 2) + 15 );
	return compound;
}
