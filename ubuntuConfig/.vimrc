set nu
set ruler
set showcmd
set mouse=v

syntax on

:set listchars=tab:>-,trail:-,extends:#,nbsp:-

set colorcolumn=101
highlight ColorColumn ctermbg=Black ctermfg=DarkRed
highlight ExtraWhitespace ctermbg=red guibg=red
match ExtraWhitespace /\s+$/
autocmd BufWinEnter * match ExtraWhitespace /\s+$/
autocmd InsertEnter * match ExtraWhitespace /\s+\%#\@<!$/
autocmd InsertLeave * match ExtraWhitespace /\s+$/
autocmd BufWinLeave * call clearmatches()

set cursorline
set cursorcolumn

highlight CursorLine   cterm=NONE ctermbg=LightBlue ctermfg=White guibg=NONE guifg=NONE
highlight CursorColumn cterm=NONE ctermbg=LightBlue ctermfg=White guibg=NONE guifg=NONE

filetype plugin indent on

set laststatus=2
set hlsearch
