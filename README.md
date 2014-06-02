# Remembrance

Remembrance is an application for tagged notes storage.
Main idea is that there is no index and each note can be accessed only by its tags.

## Usage

On the left of the window there is a list of all knows keywords (tags). They can be picked one by one, and the created set of keywords are matches all notes. If any note keyword set contains all this chosen keywords, it is considered matched and is shown below in the "Found notes" box.

Notes could be edited, removed and created. Upon note creation or edition one could pick any keyword set for that note. Some rich formatting capabilities are present, such as colour and font picking.

Database file named simply 'database' and is sought in the current working directory regardless of the application location.
Thus multiple databases could be created.

## Installation

Requires Qt4. Just run `qmake && make` and put produced `remembrance` file in your $PATH.
