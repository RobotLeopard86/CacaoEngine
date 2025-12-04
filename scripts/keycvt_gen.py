#!/usr/bin/env python3
import re
import os
import sys
import pathlib

HEADER_KEY_PATTERN = r"^\s*#define\s+(CACAO_KEY_[A-Z0-9_]+)"
DEFINE_PREFIX = "CACAO_KEY_"


# --------------------------
# Capitalization utilities
# --------------------------

def apply_capitalization(word, mode):
	"""
	Applies the selected capitalization mode:
		- 'lower' → example → 'shift'
		- 'capitalized' → 'Shift'
		- 'upper' → 'SHIFT'
	"""
	if not word:
		return word

	if mode == "lower":
		return word.lower()
	elif mode == "capitalized":
		return '_'.join([part[0].upper() + part[1:].lower() for part in word.split('_')])
	elif mode == "upper":
		return word.upper()
	else:
		return word


def apply_cap_single_letter(letter, mode):
	"""
	Applies capitalization mode to a direction *letter* (L/R).
	"""
	if mode == "lower":
		return letter.lower()
	elif mode == "capitalized":
		return letter.upper()  # single letter should probably stay uppercase
	elif mode == "upper":
		return letter.upper()
	return letter


# --------------------------
# Parsing CACAO keys
# --------------------------

def parse_header_keys(path):
	keys = []
	with open(path, "r", encoding="utf-8") as f:
		for line in f:
			m = re.match(HEADER_KEY_PATTERN, line)
			if m:
				keys.append(m.group(1))
	return keys


# --------------------------
# Pattern handling for modifiers
# --------------------------

def apply_direction_pattern(pattern, keyname, direction_word, direction_letter,
							capmode):
	"""
	Replaces:
		K → key name
		D → direction word
		d → direction letter
	"""

	out = ""

	for ch in pattern:
		if ch == "K":
			out += apply_capitalization(keyname, capmode)
		elif ch == "D":
			out += apply_capitalization(direction_word, capmode)
		elif ch == "d":
			out += apply_cap_single_letter(direction_letter, capmode)
		else:
			out += ch

	return out


# --------------------------
# Main script
# --------------------------

def main():
	header_path = pathlib.Path(sys.argv[0]).parent.parent / 'engine' / 'include' / 'Cacao' / 'Input.hpp'

	cacao_keys = parse_header_keys(header_path)
	print("WARNING: This tool is not perfect and is intended to be used only as a starting point. Please check the generated output and fix it before committing.")
	print(f"Found {len(cacao_keys)} CACAO keys.")

	control_keys = [
		'ENTER',
		'RETURN',
		'ESCAPE',
		'BACKSPACE',
		'TAB',
		'CAPS_LOCK',
		'PRINT_SCREEN',
		'SCROLL_LOCK',
		'NUM_LOCK',
		'PAUSE',
		'INSERT',
		'DELETE',
		'HOME',
		'END',
		'PAGE_UP',
		'PAGE_DOWN',
		'RIGHT',
		'LEFT',
		'UP',
		'DOWN'
	]

	print("\n=== External Library Naming Rules ===")

	# External prefix
	ext_prefix = input("External key prefix (e.g. KEY, XKB_KEY, ImGuiKey): ").strip()

	# Capitalization style
	print("\nCapitalization options: lower, capitalized, upper")
	capmode = input("How are key names capitalized? ").strip().lower()
	ctrl_capmode = input("How are control key names capitalized? ").strip().lower()

	# Keypad prefix (literal)
	kp_prefix = input(
		"Keypad prefix (literal, e.g. KP, Kp, KPAD; empty if none): "
	).strip()

	# Enter key naming
	enter_mode = input("Is the Enter key called 'enter' or 'return'? ").strip().lower()
	if enter_mode not in ("enter", "return"):
		enter_mode = "enter"

	# Modifier direction pattern
	print(
		"\nModifier direction pattern.\n"
		"Use K=key, D=direction word, d=direction letter.\n"
		"Examples: D_K, DK, K_D, K_d\n"
	)
	modifier_pattern = input("Pattern: ").strip()

	# Rename map
	print(
		"\nEnter per-key rename overrides.\n"
		"Format: CACAO_KEY_NAME external_name\n"
		"Enter an empty line when finished.\n"
	)
	rename_map = {}
	while True:
		line = input("> ").strip()
		if not line:
			break
		try:
			ck, ext = line.split()
			rename_map[ck] = ext
		except ValueError:
			print("Format must be: CACAO_KEY_NAME external_name")

	print("\n=== Generated Pairs ===\n")

	for ck in cacao_keys:
		base = ck[len(DEFINE_PREFIX):]

		# ------------------------------------
		# Overrides take priority
		# ------------------------------------
		if ck in rename_map:
			ext = rename_map[ck]
		else:
			ext = base

			# ----------------------------------------
			# ENTER special case
			# ----------------------------------------
			if base == "ENTER":
				ext = "RETURN" if enter_mode == "return" else "ENTER"

			# ----------------------------------------
			# F-keys: always preserve F#
			# ----------------------------------------
			if re.fullmatch(r"F[0-9]+", base):
				# Always uppercase F + digits
				# Apply capitalization to pattern around it only if needed,
				# but keep F# as-is.
				# Most systems use F1, F2...
				ext = base  # safe to leave unchanged

			# ----------------------------------------
			# Letter keys (single letters)
			# ----------------------------------------
			elif len(base) == 1 and base.isalpha():
				ext = apply_capitalization(base, capmode)

			# ----------------------------------------
			# Modifier keys with left/right
			# ----------------------------------------
			elif "LEFT_" in base or "RIGHT_" in base:
				if base.startswith("LEFT_"):
					direction_word = "Left"
					direction_letter = "L"
					keyname = base[len("LEFT_"):]
				else:
					direction_word = "Right"
					direction_letter = "R"
					keyname = base[len("RIGHT_"):]

				# Apply user pattern
				ext = apply_direction_pattern(
					modifier_pattern,
					keyname,
					direction_word,
					direction_letter,
					ctrl_capmode
				)

			# ----------------------------------------
			# Keypad keys
			# ----------------------------------------
			elif base.startswith("KP_"):
				suffix = apply_capitalization(base[3:], ctrl_capmode)
				if kp_prefix:
					ext = f"{kp_prefix}_{suffix}"
				else:
					ext = suffix

				# Capitalization deliberately NOT applied to kp_prefix.

			# ----------------------------------------
			# Control keys
			# ----------------------------------------
			elif base in control_keys:
				# Control key, apply capitalization with special mode
				ext = apply_capitalization(ext, ctrl_capmode)

			else:
				# Basic key, apply capitalization normally
				ext = apply_capitalization(ext, capmode)

		# Prepend external prefix
		if ext_prefix:
			final = f"{ext_prefix}_{ext}"
		else:
			final = ext

		print(f"{{{final}, {ck}}},")


if __name__ == "__main__":
	main()
