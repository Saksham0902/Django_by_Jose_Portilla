import os
# Configure settings for project
# Need to run this before calling models from application!
os.environ.setdefault('DJANGO_SETTINGS_MODULE','first_project.settings')

import django
# Import settings
django.setup()

## FAKE POP Script
import random
from first_app.models import AccessRecord,Webpage,Topic
from faker import Faker

fakegen = Faker()
topics = ['Search','Social','Marketplace','News','Games']

def add_topic():
    t = Topic.objects.get_or_create(top_name = random.choice(topics))[0]
    t.save()
    return t


def populate(N=5):
    #Create N Entries of Dates Accessed
    for entry in range(N):

        # get the topic for the entry
        top = add_topic()

        #Create the fake data for that entry
        fake_url = fakegen.url()
        fake_date = fakegen.date()
        fake_name = fakegen.company()

        #Create the new webpage entry
        webpg = Webpage.objects.get_or_create(topic=top,url=fake_url,name=fake_name)[0] #get_or_create()- returns a tuple and [0] gives first element of the tuple

        #create a fake access record for that Webpage
        # Could add more of these if you wanted...
        acc_rec = AccessRecord.objects.get_or_create(name=webpg,date=fake_date)[0]

if __name__ == '__main__':
    print("Populating the databases...Please Wait")
    populate(20)
    print('Populating Complete')
