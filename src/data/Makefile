# Makefile for Multiarcade data files.

DATA_MIDDIR:=mid/data

# Find the input files and sort by type (must be in a typed directory).
DATA_SRCFILES:=$(shell find src/data -type f)
DATA_SRCFILES_IMAGE:=$(filter src/data/image/%,$(DATA_SRCFILES))
DATA_SRCFILES_FONT:=$(filter src/data/font/%,$(DATA_SRCFILES))
DATA_SRCFILES_SONG:=$(filter src/data/song/%,$(DATA_SRCFILES))
DATA_SRCFILES_SOUND:=$(filter src/data/sound/%,$(DATA_SRCFILES))
DATA_SRCFILES_RAW:=$(filter src/data/raw/%,$(DATA_SRCFILES))

# Run splashes through mktsv, we want raw 8-bit images with no header.
# TODO Clean this up. (clean up "raw" data in general...)
DATA_SPLASH_SRC:=$(filter src/data/raw/Sitter/splash/%.png,$(DATA_SRCFILES_RAW))
DATA_SPLASH_DST:=$(DATA_SPLASH_SRC:.png=.tsv)
all:$(DATA_SPLASH_DST)
src/data/raw/Sitter/splash/%.tsv:src/data/raw/Sitter/splash/%.png $(EXE_mktsv);$(PRECMD) $(EXE_mktsv) -o$@ $< -s8

# DATA_SRCFILES: Inputs that require some kind of pre-processing.
DATA_SRCFILES:=$(DATA_SRCFILES_IMAGE) $(DATA_SRCFILES_FONT) $(DATA_SRCFILES_SONG) $(DATA_SRCFILES_SOUND)

DATA_DSTFILES:=$(patsubst src/data/%,$(DATA_MIDDIR)/%.c,$(basename $(DATA_SRCFILES)))

$(DATA_MIDDIR)/image/%.c:src/data/image/%.png $(EXE_mktex);$(PRECMD) $(EXE_mktex) -o$@ $<
$(DATA_MIDDIR)/font/%.c:src/data/font/%.png $(EXE_mkfont);$(PRECMD) $(EXE_mkfont) -o$@ $<
$(DATA_MIDDIR)/song/%.c:src/data/song/%.mid $(EXE_mksong);$(PRECMD) $(EXE_mksong) -o$@ $<

DATA_TSV:=out/$(PROJECT_NAME).tsv
$(DATA_TSV):src/data/title.png $(EXE_mktsv);$(PRECMD) $(EXE_mktsv) -o$@ $<
