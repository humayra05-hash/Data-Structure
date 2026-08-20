# DS_Project
It is the 3rd semester Data Structure course project of our University named Lexicon LookUp Engine.
We have completed the project successfully under the supervision of our course teacher Jamilul Huq Jami sir.

## Repository Outline:
->lexi_log.c (It is main source code file of our Project coded in C programming Language)
->lexicon_db.c (It is the Database file of our project)
->Lexicon Engine Slides (It is the PDF format file of our Presentation Slides)
->Project Report Lexicon Engine (It is the fully completed project report file in PDF format)

## Our team: 
1.Bayezid Bostami (253-15-442)
2.Nabila Mehzabin (253-15-162)
3.Humayra Mehzabin (253-15-163)
4.Md. Mahinur Rahman Khan (253-15-151)
5.Sanjidur Rahman (253-15-662)


## Project Overview: -> Lexicon LookUp Engine <-
This project is a C-based Lexicon Application System designed to perform dictionary management operations using foundational and advanced data structures.The system incorporates a user authentication model where users can register new accounts and log-in using persistent storage maintained in a user database file. Upon successful login,the application loads lexicon data from a dedicated database file into memory to handle word lookup, insertion and deletion operations dynamically.The core architecture relies on an array of efficient data structures to manage data access and maintain performance.A master singly linked list retains the primary entry nodes,while a self-balancing AVL tree functions as an explicit set to guarantee fast O(log n) lookup times. Additionally,a standard Binary Search Tree (BST) is maintained alongside the AVL tree to generate real-time comparison metrics during search operations.To enhance user experience,the application utilizes a Queue structure to track the last ten search queries as history and a Stack structure to support an interactive Undo mechanism for recent insertions or deletions.
