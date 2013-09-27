## AMiner::DataBase Tables

### AMiner.org Database Tables

This file describs aminer tables.

  * LAGEND
    * [?] - unknown
    * [x] - not used, should delete.
    * [.] - used online, modified online.
    * [s] - used online, offline generated table.
    * [o] - other useful, used not online but some other place.
    * [d] - not finished function, in development.

#### Important Tables
 PROXY                  | . - 在线添加paper的工具，需要访问googlescholar，其中使用的代理列表。（作者董玉冰. 需要重写，最好是独立程序）
 admin_import_history   | . - 数据导入工具， paper在线添加工具的历史记录。                     
 admin_import_item      | ? - empty, nouse                                                                              
 admin_import_history   | . - 数据导入工具， paper在线添加工具的历史记录。                     
 admin_import_item      | ? - empty, nouse                                                     
 ap_register            | . - APP register, 注册AMinerAPP                                      
 ap_user_selection      | . - app used in user's portal(search resule)                        
 app_poll               | . - poll system                                                     
 app_poll_log           | . - poll system log                                                  
 app_workshop_block     | . - ArnetPage - a simple wiki system.                                
 app_workshop_page      | . - ArnetPage page                                                   
 author_affilation      | s - From ACM Web, author-paper-affiliation                           
 author_links           | . - Author links in Profile page, twitter etc...                     
 bole_relation          | s - adivser advisee relation. OLD (by name only)                     
                        |                                                                      
 citation               | s - citation relation, by publication.pubkey                         
 citation_raw           | o - citation string that can not parsed.                             
 comments               | . - ?                                     
 conference_events      | x - from wikicfp, notused now, TODO                                  
 conference_info        | x - conference info, shound not use.      
 conference_rank        | s - Conference Ranks, old data. 2000      
 constants              | . - system constants                      
 contact_info           | . - ContactInfo of Author                 
 contact_mod_audit      | . - Contact Modification                                             
 contact_mod_history    | . - ..                                    
 course_page            | s - Course search APP                     
 emailsettings          | . - P/notification email settings.        
 jconf                  | . - core-table, conference / journals     
 na_author2pub          | . - core-table, author paper relation     
 na_paper_organization  | s - organization app                      
 na_person              | . - core-table, author                    
 na_person_geo          | s - location search app                   
 na_person_organization | s - organization app                      
 na_person_relation     | . - coauthor relation                     
                        |                                           
 notification           | . - notification system.                  
                        |                                           
 organization           | s - organization app                      
 organization_geo       | s - organization app                      
                        |                                           
 pdf_PdfRating          | . - pdfviewer used                        
 pdf_Rating             | . - ..                                    
 pdf_Tag                | . - ..                                    
 pdf_UserTag            | . - ..                                    
 pdf_annotation         | . - ..                                    
 pdf_history            | . - ..                                    
 pdf_like               | . - ..                                    
                        |                                           
 person_interest        | . - Author's interests                    
 person_update_ext      | o - used in GoogleScholar Citation Crawller.                         
 post                   | . - p/post                                
 post_like_log          | . - p/post like                           
 pub_type               | . - core-table                            
 publication            | . - core-table                            
 publication_ext        | . - core-table, publication.abstract      
 publication_pdf        | . - core-table, publication pdf file location.                       
                        |                                           
 publisher              | s - publisher, update data but not used online.                      
 reviewer_feedback      | . - subproject::reviewer system           
 reviewer_paper         | . - ..                                    
                        |                                           
 sequence               | . - system sequence.                      
 tag                    | . p/tag                                   
 tag_vote_log           | . p/tag vote                              
                        |                                           
 toppaper               | . ConferenceBestPaper http://arnetminer.org/conferencebestpapers     
                        |                                           
 url_rewrite            | . - url rewrite mapping. e.g.: /download -> ...                      
 user2author            | . - bind author info.                     
                        |                                           
 user_suggest           | . feedback data                           
 user_tag_links         | . user taglinks                           
 users                  | . core-table, users                       
                        |                                           
 viewer_author          | . pdf viewer system.                      
 viewer_citation        |                                           
 viewer_comment         |                                           
 viewer_line            |                                           
 viewer_neighbor        |                                           
 viewer_paper           |                                           
 viewer_recommendation  |                                           
 viewer_weight          |                                           
 viewer_word            |                                           


#### Log Tables
 log_association_graph             | . - logs
 log_share2sns                     | . - logs
 history                           | . - many operation's history
 na_mod_pub_log                    | . - log
 change_record                     | x - Very old log table, not used


#### Not Used Table
 ORGANIZATION                      | ?                     
 ORGANIZATION_na_person            | ?                     
 _affilation                       | x                     
                               
 advisor_nation_score              | ？                    
 advisor_recommender_feedback      | ?                     
 advisor_recommender_user          | ?                     
 advisor_relation                  | ?                     
                               
 app_pdf_paper                     | ?                     
 arnetapp_application              | ?                     
 arnetapp_userselection            | ?                     
                               
 author_affilation_new             | ?                     
 author_googlescholar_profile_link | ?                     
                               
 bole_feature                      | ? - not used?         
                               
 comparegroup                      | - _____ notfinished ______                       |
 comparegroup_relation             | - ..                  
                               
 calendar_event                    | - - Yutao Zhang's calendar system                |
 calendar_record                   | -                     
                               
 du_na_person_annotated            | x                     
 du_na_pub_annotated               | x                     
                               
 feedback                          | x - feedback system // TODO need rewrite         |
 feedback2                         | x - feedback system // TODO need rewrite         |
                               
 follows                           | x - notused, use redis to store follows relation |
 hindex  ?                     
                               
 na_author_map                     | ?                     
                               
 logrecord                         | x - very old log table.                          |
                               
 notification_notifier             | x - replaced by redis 
 notifications                     | x - replaced by redis 
 notifier                          | x - replaced by redis 
 nsf     x - NSF information   
 nsfcn   x - China NSF inforamtion                        |
                               
 person  x - old no Disambiguationed data, not update.    |
 person_ext                        | x - person statistic info, replaced by redis.    |
 person_ext_top                    | x - ..                
                               
 publication_pro                   | x                     
 publication_ref                   | x                     
 publication_url                   | x                     
                               
 s_3score                          | x                     
 s_contact_info                    | x                     
 topsearch                         | x                     
 university                        | x                     
 uploadfile                        | x                     
