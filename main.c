//
// Created by Andre Natal on 6/30/15.
// anatal@mozilla.com
//

#include "main.h"
#include <sphinxbase/err.h>
#include <sphinxbase/glist.h>
#include <sphinxbase/ckd_alloc.h>
#include <sphinxbase/strfuncs.h>

int main() {
    char word_grapheme[] = "teobaldo";
    char *result = g2p(word_grapheme);
    printf("-%s-\n" , result);

   // char word_grapheme2[] = "sandip";
   // char *result2 = g2p(word_grapheme2);
    //printf("%s \n" , result2);

    return 0;
}

char * g2p(char word_grapheme[])
{
    err_set_logfp(NULL);
    logmath_t *logmath = logmath_init(1.0001f, 0, 0);
    ngram_model_t *model = ngram_model_read(NULL,"/Users/anatal/Downloads/g2p-model/cmudict-g2p.lm.dmp",NGRAM_AUTO,logmath);
    const int32 *total_unigrams = ngram_model_get_counts(model);
    int32 history[1000];
    int32 winner_wid = 0;

    // start with sentence
    int32 wid_sentence = ngram_wid(model,"<s>");
    history[0] = wid_sentence;
    int totalh = 0;
    size_t increment = 1;
    glist_t list_wngram = NULL;
    int sz_list_wngram = 0;

    int offset_word = 0;
    for (int j = 0 ; j <= (int) strlen(word_grapheme)-1 ; j += increment) {
        //char letter = 'l' ;//word_grapheme[j];
        //char letter = word_grapheme[j];

        //printf("letter %c %i \n" , letter, j);

        struct winner_t winner_s = get_winner_wid(model,word_grapheme,history,*total_unigrams,totalh,offset_word);

        //printf(" letter %c winner id %i \n" , letter,  winner_wid);
        increment = winner_s.length_match;
        history[j+1] = winner_s.winner_wid;
        totalh = j+1;
        offset_word += winner_s.length_match;
    }

    for (int w = 0; w <= totalh; w++){
        const char* word;
        word = ngram_word(model, history[w]);

        char *new_vocab;
        new_vocab = strdup(word);
        char *saveptr;

        const char *pch = strtok_r(&new_vocab[0], "}", &saveptr);
        pch = strtok_r(NULL, "}", &saveptr);

        if (pch == NULL)
            continue;

        list_wngram = glist_add_ptr(list_wngram, (void*)pch);
    }

    free(logmath);
    ngram_model_free(model);

    int size_list_wngram = glist_count(list_wngram);
    char const *final_word = ckd_calloc ( 1, size_list_wngram);

    struct gnode_s *gnode_item =  glist_tail(list_wngram);
    final_word = string_join(final_word,  (char*)gnode_ptr(gnode_item) , NULL);

    list_wngram = glist_reverse(list_wngram);

    for (int i = 0 ; i <= size_list_wngram+1; i++)
    {
        gnode_item = gnode_next(gnode_item);
        if (gnode_item == NULL)
            break;
        final_word = string_join(final_word, " " , (char*)gnode_ptr(gnode_item)  , NULL);
    }

    glist_free(list_wngram);
    return final_word;
}



struct winner_t get_winner_wid(ngram_model_t *model, char word_grapheme[], int32 history[], const int32 total_unigrams, int total_history, int offset_word) {
    //printf("letter %s total unigrams %i \n" , letter, total_unigrams);
    //printf("letra aqui %c " , letter);

    int32 winner_wid = 0;
    int32 current_prob = -2147483647;
    struct winner_t winner_s; //Treating it like a normal variable type

    for (int32 i = 0; i <= total_unigrams; i++) {

        const char *vocab = ngram_word(model, i);

        if (vocab == NULL)
            continue;

        char *new_vocab;
        new_vocab = strdup(vocab);
        char *saveptr;

        char *pch = strtok_r(new_vocab, "}", &saveptr);

        char* str_pch = malloc(strlen(pch)+1);
        strcpy(str_pch, pch);
        removeChar(str_pch, '|');

        char sub[1000];
        substring(word_grapheme, sub, offset_word+1, (strlen(word_grapheme) - offset_word));
        char *remainder = word_grapheme + offset_word;
        //printf("-%s-%s-%s-%s-\n", vocab, str_pch,pch,sub);
        if ( startsWith(str_pch, sub)){
            int nused;
            const int32 prob = ngram_ng_prob(model, i, history, total_history, &nused);

            //printf("matched:%i-%s-%s-%s-%s-%i-\n", i, vocab, str_pch,pch,sub,prob);

            if (current_prob < prob) {
                current_prob = prob;
                winner_s.winner_wid = i;
                winner_s.length_match = strlen(pch);

                //printf("winner %s - %s - %s \n", vocab, str,pch);

            }


        }



        free(str_pch);
        free(new_vocab);

    }
    //printf ("winner %i %i \n" , winner_s.winner_wid, winner_s.length_match);
    return winner_s;
}


int startsWith(const char *pre, const char *str) {
    size_t lenpre = strlen(pre), lenstr = strlen(str);
    return lenstr < lenpre ? 0 : strncmp(pre, str, lenpre) == 0;
}

void removeChar(char *str, char garbage) {
    char *src, *dst;
    for (src = dst = str; *src != '\0'; src++) {
        *dst = *src;
        if (*dst != garbage) dst++;
    }
    *dst = '\0';
}

void substring(char s[], char sub[], int p, int l) {
    int c = 0;

    while (c < l) {
        sub[c] = s[p+c-1];
        c++;
    }
    sub[c] = '\0';
}