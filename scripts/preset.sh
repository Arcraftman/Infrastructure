#!/bin/sh
ctags -R --exclude=node_modules \
	 --exclude=.git \
	 --exclude=build \
	 --exclude=doc \
	 --exclude=scripts \
	 --exclude=.vscode .


