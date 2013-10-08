import redis

def get_author_statistics():
	redis_client = redis.StrictRedis(host="10.1.1.110", db=0)
    redis_client.zrevrange("AuthorFeature:hindex:t"+str(t),0,lim)
	redis_client.zrevrangebyscore("AuthorFeature:hindex",min=0,max=1000,withscores=True)

def get_author_statistics_from_db():
	import MySQLdb
	db = MySQLdb.connect(host="10.1.1.110",
						 user="root",
						 passwd="keg2012",
						 db="arnet_db")
	cur = db.cursor()
	SQL_PERSON_FEATURE = "select person_id, type, score, rank from person_ext where topic=-1 and type<10"
	cur.execute(SQL_PERSON_FEATURE)
	result = cur.fetchall()
	features = [{} for i in range(10)]
	sorted_features = [None for i in range(10)]
	print "start"
	idx = 0
	for item in result:
		if idx % 10000==0:
			print idx
		idx += 1
		features[item[1]][item[0]] = item[2]
	for i in range(10):
		sorted_features[i] = sort_dict(features[i])
	dump_data(features,"features")
	dump_data(features,"sorted_features")



def load_data(name):
    import pickle
    data = None
    with open("/home/yutao/data/aminer/"+name+".pickle","rb") as f_out:
        data = pickle.load(f_out)    
    return data

def dump_data(data, name):
    import pickle
    with open("/home/yutao/data/aminer/"+name+".pickle","wb") as f_out:
        pickle.dump(data, f_out)

def sort_dict(data):
	return sorted(data.items(),key=lambda x:x[1],reverse=True)