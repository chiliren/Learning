set nu
set ruler
set showcmd
set mouse=v

syntax on

set colorcolumn=101
highlight ColorCol0umn ctermbg=Black ctermfg=DarkRed
highlight ExtraWhitespace ctermbg=red guibg=red
match ExtraWhitespace /\s+$/
autocmd BufWinEnter * match ExtraWhitespace /\s+$/
autocmd InsertEnter * match ExtraWhitespace /\s+\%#\@<!$/
autocmd InsertLeave * match ExtraWhitespace /\s+$/
autocmd BufWinLeave * call clearmatches()

"设置横竖对齐的光柱
set cursorline
set cursorcolumn
highlight CursorLine   cterm=NONE ctermbg=LightBlue ctermfg=White guibg=NONE guifg=NONE
highlight CursorColumn cterm=NONE ctermbg=LightBlue ctermfg=White guibg=NONE guifg=NONE

filetype plugin indent on

"vim中显示文件名
set laststatus=2 
set hlsearch
