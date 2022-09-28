#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * externs
 */
typedef struct yy_buffer_state* YY_BUFFER_STATE;
extern int yyparse();
extern YY_BUFFER_STATE yy_scan_string(const char * str);
extern void yy_delete_buffer(YY_BUFFER_STATE buffer);

/*
 * in file global variables static
 */
static FILE *out_fp;

struct params {
	char* input_file_path;
	char* output_file_path;
	int   quote_type;
};
static struct params s_params = {NULL, NULL, -1};

struct kv {
	char* 	 key;
	uint32_t value;
};
#define KV_MAX 256
static struct kv s_key_value[KV_MAX] = {0};

static uint32_t s_printing_line_num = 0;

static uint32_t s_chapter_num = 0;
static uint32_t s_code_num_of_chap = 0;
static uint32_t s_figure_num_of_chap = 0;

static int s_quote_toggle = 0;
static int s_recording_code_line = 0;
static int s_writing_plain = 0;

/*
 * private functions
 */
static void  show_usage(const char* exefilename);
static void  parse_args(int argc, char* argv[]);
static char* get_code_quote(int start_end);

static void add_key_value(char* key, uint32_t value);
static uint32_t get_key_value(const char* key);
static uint32_t search_key(const char* key);

/*
 * macros
 */
#define SET_2_CH_KEY(s)	char key[3] = {(s)[0], (s)[1], 0}
#define SET_3_CH_KEY(s) char key[4] = {(s)[0], (s)[1], (s)[2], 0}

int main(int argc, char* argv[])
{
	FILE *fp;

	parse_args(argc, argv);

	fp = fopen(s_params.input_file_path, "r");
	out_fp = fopen(s_params.output_file_path, "w");

	char *line = NULL;
	size_t len = 0;
	ssize_t nread;

	while ((nread = getline(&line, &len, fp)) != -1) {
		if (line[0] == '\n') {
			continue;
		}
		YY_BUFFER_STATE yyst = yy_scan_string(line);
    	int parse_result = yyparse();
    	yy_delete_buffer(yyst);
	}

	if (line) {
		free(line);
	}

	fclose(out_fp);
	fclose(fp);

	return 0;
}

void write_header(const char* str)
{
	if (str[0] == '#' && str[1] == ' ') {
		s_chapter_num++;
		s_code_num_of_chap = 0;
		s_figure_num_of_chap = 0;
	}
	fprintf(out_fp, "%s\n", str);
}

void write_text(const char* str)
{
	// cosmetic purpose. if not use below, there are no empty line between plain and text
	if (s_writing_plain) {
		fprintf(out_fp, "\n");
		s_writing_plain = 0;
	}

	if (str) {
		fprintf(out_fp, "%s\n", str);
	}
}

void write_code_line(const char* str)
{
	if (s_printing_line_num) {
		fprintf(out_fp, "%02d|%s", s_printing_line_num++, str);	
	} else {
		fprintf(out_fp, "%s", str);
	}
}

void write_replace_code_line(const char* str)
{
	SET_2_CH_KEY(str);
	if (s_recording_code_line) {
		add_key_value(key, s_printing_line_num);
		fprintf(out_fp, "%02d|%s", s_printing_line_num++, &str[3]);
	} else {
		uint32_t line_num = get_key_value(key);
		if (line_num == 0) {
			add_key_value(key, s_printing_line_num);
			fprintf(out_fp, "%02d|%s", s_printing_line_num++, &str[3]);
			return;
		}
		fprintf(out_fp, "%02d|%s", line_num, &str[3]);
		// if s_printing_line_num is not zero means that second position so, reset to zero
		// if s_printing_line_num is zero means that first position so, set the starting line number
		s_printing_line_num = s_printing_line_num ? 0 : (line_num + 1);
	}
}

void write_code_num(const char* ref)
{
	s_code_num_of_chap++;
	SET_3_CH_KEY(&ref[5]);
	uint32_t value = (s_chapter_num * 100) + s_code_num_of_chap;
	add_key_value(key, value);
	fprintf(out_fp, "[Code %02d-%02d]\n", s_chapter_num, s_code_num_of_chap);
}

void write_figure_num(const char* ref)
{
	s_figure_num_of_chap++;
	SET_3_CH_KEY(&ref[7]);
	uint32_t value = (s_chapter_num * 100) + s_figure_num_of_chap;
	add_key_value(key, value);
	fprintf(out_fp, "[Figure %02d-%02d]\n", s_chapter_num, s_figure_num_of_chap);
}

void write_quote(void)
{
	char* quote = get_code_quote(s_quote_toggle);
	fputs(quote, out_fp);
	s_quote_toggle = !s_quote_toggle;
}

void write_start_code_line(void)
{
	write_quote();
	s_recording_code_line = !s_recording_code_line;
	s_printing_line_num = s_printing_line_num ? 0 : 1;
}

void write_plain(const char* str)
{
	fprintf(out_fp, "%s", str);
	s_writing_plain = 1;
}

uint32_t get_val_as_2key(char k1, char k2)
{
	char key[3] = {0};
	key[0] = k1;
	key[1] = k2;
	return get_key_value(key);
}

uint32_t get_val_as_3key(char k1, char k2, char k3)
{
	char key[4] = {0};
	key[0] = k1;
	key[1] = k2;
	key[2] = k3;

	return get_key_value(key);
}

static void show_usage(const char* exefilename)
{
	fprintf(stderr, "Usage: %s -i input_path -o output_path -t quote_type\n",
			exefilename);
	fprintf(stderr, "\tquote_type: 0 - my blog (```)\n");
	fprintf(stderr, "\t            1 - kldp    (<code>)\n");

}

static void parse_args(int argc, char* argv[])
{
	int opt;
	while ((opt = getopt(argc, argv, "i:o:t:")) != -1) {
		switch(opt) {
			case 'i':
				s_params.input_file_path = strdup(optarg);
				break;
			case 'o':
				s_params.output_file_path = strdup(optarg);
				break;
			case 't':
				s_params.quote_type = atoi(optarg);
				break;
			default: // '?'
				show_usage(argv[0]);
				exit(-1);
		}
	}

	if (s_params.input_file_path == NULL ||
			s_params.output_file_path == NULL ||
			s_params.quote_type == -1) {
		show_usage(argv[0]);
		exit(-1);
	}
}

static char* get_code_quote(int start_end)
{
	char* quote;

	switch(s_params.quote_type) {
		case 0:
			if (start_end == 0) {
				quote = "{% highlight c %}\n";
			} else {
				quote = "{% endhighlight %}\n\n";
			}
			break;
		case 1:
			if (start_end == 0) {
				quote = "<code>\n";
			} else {
				quote = "</code>\n";
			}
		case 2:
			if (start_end == 0) {
				quote = "```\n";
			} else {
				quote = "```\n";
			}
	}

	return quote;
}

#define NOT_FOUND(idx) (1 << 16 | idx)
#define IS_NOT_FOUND(idx) (idx >> 16)
#define GET_LAST_IDX(idx) (idx & 0xffff)

static void add_key_value(char* key, uint32_t value)
{
	uint32_t key_index = search_key(key);
	if (IS_NOT_FOUND(key_index)) {
		s_key_value[GET_LAST_IDX(key_index)].key = strdup(key);
		key_index = GET_LAST_IDX(key_index);
	}
	s_key_value[key_index].value = value;
}

static uint32_t get_key_value(const char* key)
{
	uint32_t key_index = search_key(key);
	return (IS_NOT_FOUND(key_index) ? 0 : s_key_value[key_index].value);
}

static uint32_t search_key(const char* key)
{
	// Just linear search
	for (uint32_t i = 0 ; i < KV_MAX ; ++i)
	{
		if (s_key_value[i].key == NULL) {
			return NOT_FOUND(i);
		}
		if (strncmp(s_key_value[i].key, key, strlen(key)) == 0) {
			return i;
		}
	}

	fprintf(stderr, "Not enough key-value storage\n");
	exit(-1);

	return (uint32_t)-1;
}

void hex_debug(const char* str)
{
	for (int i = 0 ; i < strlen(str) ; i++) {
		printf("%02x ", (char)(str[i] & 0xff));
	}
}
