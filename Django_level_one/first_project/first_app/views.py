from django.shortcuts import render
from django.http import HttpResponse
from first_app.models import Topic,Webpage,AccessRecord
# Create your views here.
def index(request):
    webpages_list = AccessRecord.objects.order_by('date')#order_by - sql command that refers to order according to given argument
    date_dict = {'access_records':webpages_list}
    return render(request,'first_app/index.html',context=date_dict)
