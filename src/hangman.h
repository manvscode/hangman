#ifndef _HANGMAN_H_
#define _HANGMAN_H_

struct hangman;
typedef struct hangman hangman_t;

hangman_t* hangman_create(const char* dictionary[], size_t dictionary_size);
void hangman_destroy(hangman_t** game);
void hangman_run(hangman_t* game);
void hangman_resize(hangman_t* game);

#endif /* _HANGMAN_H_ */
