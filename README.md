## SAE -- Supporting AMiner -- Plan
Jianfei Wang, Yutao Zhang, Yaohui Ye
2013/09/23

### Target
#### Short term
- Replace AMiner's current DataCenter.

- Provide a fast (both in terms of startup speed and serving speed), memory-efficient (require little preloading), and elegant (easy to use and modify) backend server for AMiner.


#### Long term
- Act as a general purpose graph database, providing CRUD+Search for the frontend?

#### People
- Yutao Zhang
- Yaohui Ye, chief programmer
- Jianfei Wang, chief architect


#### Plan
- make a concrete plan--before 23rd
- Core read-only services (2 weeks) -- before Oct. 14th
- Advanced search and apps (2 weeks) -- before Nov. 1st
- Data updates (2 weeks) -- before Nov. 15th
- Testing (2 weeks) -- before Dec. 1st


##### Stage 1 (expert finding and core features) -- Oct. 21th (3weeks * 3 person)

Simple data import (from MySQL and existing ACT model and Redis and so on), index building

- ACT (load model, infer new doc, ACT services for AMiner)   
	- ACTService    getParameter   
	- ACTService    getTopicDistributionGivenAuthorNAid   
	- ACTService    getTopicDistributionGivenAuthorName   
	- ACTService    getTopicDistributionGivenConfId
	- ACTService    getTopicDistributionGivenConfName
	- ACTService    getTopicDistributionGivenPub
	- ACTService    getTopicDistributionGivenQuery
	- ACTService    getAuthorDistributionGivenTopic
	- ACTService    getConfDistributionGivenTopic
	- ACTService    getPubDistributionGivenTopic
	- ACTService    getWordDistributionGivenTopic


- Author Features (like Redis zset)
	- getScore(author, feature, topic)
	- getRank(author, feature, topic)
	- topAuthors(feature, topic, from, to)
	- count(feature, topic)
	- getMinValue/getMaxValue(feature, topic)
	- getAuthorFeature(author, topic)
- Basic search
	- AuthorService    searchAuthors
		- merge: getAuthorsBy{Id, Name}
		- merge: getAuthorsWithSameNameBy{Id,Name}
 	- ConfService    searchConferences
		- merge: getConferenceBy{Id,Name,Prefix}
	- PubService    searchPublications
		- merge: searchPublicationsWithScore
		- merge: getPublicationsBy{Id, NaId, ConfId}
		- merge:  getCitedByPulicataions
		- merge:  getCitePublications
	- OrgService    searchOrganizations
		- merge: getOrganizationsBy{Id,Name,Prefix}
	- Suggestion services
		- AuthorService authorSearchSuggest
	- Modify services
		- AuthorService updateAuthorProfile


##### Stage 2 (full expert search experience, support AMiner frontend) Nov. 1st
- Advanced search (advanced query parsing, result filtering)
- Graph search (http://arnetminer.org/association-home)
- Reviewer recommendation (?) -- Wanlin Hong
- Collaborator recommendation
- Paper recommendation (?)
- PubService    recommendRelatedPaper

##### Stage 3 (data updates) Nov. 15th
- Add/update/remove publications 
- Add/merge/remove authors
- ACT model update
- Author features update (remove Redis dependency)

##### Stage 4 (testing and performance tuning) Dec. 1st
- Detailed smoke testing plan and workflow
- Performance testing plan and results

#### Appendix A -- List of renamed AMiner services
Those getters are merged into search services, by special queries like: “naid:[10000, 10001]”.

// merged into searchConferences

- ConfService    getConferenceById   
- ConfService    getConferenceByName   
- ConfService    getConferenceByPrefix    

// merged into searchOrganizations

- OrgService    getOrgnizationById   
- OrgService    getOrganizationByPrefix

// merged into searchAuthors
AuthorService    getAuthorsByName
AuthorService    getAuthorsById
AuthorService    getAuthorsWithSameNameById
AuthorService    getAuthorsWithSameNameByName

// merged into searchPublications

- PubService    getPublicationsById
- PubService    getPublicationsByNaId
- PubService    getPublicationsByConfId
- PubService    getCitedByPulicataions
- PubService    getCitePublications

#### Appendix B -- List of removed AMiner services

Most the removed services are relatedcd ..
 to graph/content modifications.

// Various ranking services, should be moved to Redis? Or?

- OrgService    getTopHindexAuthors
- OrgService    getTopicTrend
- OrgService    getHotTopics

// How do we implement these features?

- AuthorService    addUpdateAuthor
- AuthorService    movePublication
- AuthorService    searchAuthorProfile
- AuthorService    reloadAuthors
- AuthorService    removeAuthor
- AuthorService    recalcuFeatures

- PubService    addPublication
- PubService    reviseAuthorIdNameMatches
- PubService    inferAuthorIdsOfPublication
- PubService    removePublications
- PubService    reloadPublications
- PubService    updatePublication

// how about other search services?--jie

- a. search filtering (like the func in SAE-HR)
- b. geographic search 


