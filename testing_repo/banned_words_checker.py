import argparse
import json
import sys


def main():
    parser = argparse.ArgumentParser(description='Check for banned words')
    parser.add_argument('--solution', type=str, required=True, dest='solution_path')
    parser.add_argument('--banned-words', type=str, required=True, dest='banned_words_json_path')
    parser.add_argument('--delimiters', type=str, dest='delimiters_json_path')
    args = parser.parse_args()

    with open(args.banned_words_json_path, 'r') as banned_words_json_file:
        banned_words_json = json.load(banned_words_json_file)
        banned_words = banned_words_json.get('banned_words', [])

    with open(args.solution_path, 'r') as solution_file:
        file_content = solution_file.read()

    if args.delimiters_json_path is not None:
        delimiters = json.load(open(args.delimiters_json_path, 'r'))
    else:
        delimiters = [' ', ',', '.', '\t', ';', '(', ')', '{', '}', '[', ']',
                      '&', '*', '!', '=', '+', '-', '/', '%', '@', '\\', '^']

    for delimiter in delimiters:
        file_content = file_content.replace(delimiter, f' {delimiter} ')
    tokens = set(file_content.split())

    for word in banned_words:
        if word in tokens:
            sys.exit(f'Word {word} is banned!')


if __name__ == '__main__':
    main()
