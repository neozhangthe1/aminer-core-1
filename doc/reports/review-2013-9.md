## Monthly Review 
##### AMiner Group - Yutao


#### Tasks:

- Google Scholar Crawler

		https://github.com/neozhangthe1/aminer-spider

	The general goal is to replace the old and unmaintainable Google scholar crawler with a light wight, clean, fast and flexible one. Currently we have implemented the new crawler base on a 3rd party scraping framework with basic crawling and storing function. We will focus on improving the performance of paper matching between AMiner and Google Scholar in the following week.
	

- AMiner to SAE interface

		https://github.com/neozhangthe1/aminer-core

	The general goal is to replace AMiner's current DataCenter. Provide a fast (both in terms of startup speed and serving speed), memory-efficient (require little preloading), and elegant (easy to use and modify) backend server for AMiner. 
	
	I've finished reconstruct the structure of the project and some code cleaning work, and started to implement the data importer.


- Dynamic Expert Search

	The idea is to enable expert finding given any time point. Linxi is working on obtaining ground truth for evaluation. We still need a formal and concrete definition of the problem, which is the primary mission in the following week.
	 

- Entity Search with Online Learning

		https://docs.google.com/document/d/1dqtSNwL-z591juHYA6LcaIKtVPoTWaG8x_WEltTSvNc/

	This work is collaborating with Jianfei and Qianru. The problem has been formally defined in the google doc. I'll provide help for integrating the function with AMiner.
	


#### Members:   

- Jianyuan Lai:
	
	He is in charge of re-implementing the Google scholar crawler. He is not a experienced programmer as he took some time to getting familiar with the programming language and the framework, but fortunately he is willing to learn. He has implemented the crawler with base functions and is starting to working on paper matching between two google scholar and AMiner.

- Linxi Zou:

	She will be working on Dynamic Expert Search. Since she's just entering our lab, so there's not much to comment. She is collecting PC members of each conference as the ground truth for evaluation.